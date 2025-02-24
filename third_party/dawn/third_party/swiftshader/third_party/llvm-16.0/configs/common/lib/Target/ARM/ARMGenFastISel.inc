/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* "Fast" Instruction Selector for the ARM target                             *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/


// FastEmit Immediate Predicate functions.
static bool Predicate_mod_imm(int64_t Imm) {

    return ARM_AM::getSOImmVal(Imm) != -1;
  
}
static bool Predicate_imm0_65535(int64_t Imm) {

  return Imm >= 0 && Imm < 65536;

}
static bool Predicate_imm0_7(int64_t Imm) {

  return Imm >= 0 && Imm < 8;

}
static bool Predicate_imm8_255(int64_t Imm) {

  return Imm >= 8 && Imm < 256;

}
static bool Predicate_imm0_255(int64_t Imm) {
 return Imm >= 0 && Imm < 256; 
}
static bool Predicate_t2_so_imm(int64_t Imm) {

    return ARM_AM::getT2SOImmVal(Imm) != -1;
  
}
static bool Predicate_imm0_4095(int64_t Imm) {

  return Imm >= 0 && Imm < 4096;

}
static bool Predicate_imm1_31(int64_t Imm) {
 return Imm > 0 && Imm < 32; 
}
static bool Predicate_shr_imm8(int64_t Imm) {
 return Imm > 0 && Imm <= 8; 
}
static bool Predicate_shr_imm16(int64_t Imm) {
 return Imm > 0 && Imm <= 16; 
}
static bool Predicate_shr_imm32(int64_t Imm) {
 return Imm > 0 && Imm <= 32; 
}
static bool Predicate_VectorIndex32(int64_t Imm) {

  return ((uint64_t)Imm) < 2;

}
static bool Predicate_imm0_31(int64_t Imm) {

  return Imm >= 0 && Imm < 32;

}
static bool Predicate_t2_so_imm_neg(int64_t Imm) {

  return Imm && ARM_AM::getT2SOImmVal(-(uint32_t)Imm) != -1;

}
static bool Predicate_imm0_15(int64_t Imm) {

  return Imm >= 0 && Imm < 16;

}


// FastEmit functions for ARMISD::CALL.

unsigned fastEmit_ARMISD_CALL_MVT_i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->hasV5TOps()) && (!Subtarget->isThumb()) && ( MF->getSubtarget<ARMSubtarget>().hardenSlsBlr() )) {
    return fastEmitInst_r(ARM::BLX_noip, &ARM::GPRnoipRegClass, Op0);
  }
  if ((Subtarget->hasV5TOps()) && (!Subtarget->isThumb()) && ( !MF->getSubtarget<ARMSubtarget>().hardenSlsBlr() )) {
    return fastEmitInst_r(ARM::BLX, &ARM::GPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_CALL_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_CALL_MVT_i32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::CALL_NOLINK.

unsigned fastEmit_ARMISD_CALL_NOLINK_MVT_i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->isThumb()) && (Subtarget->isThumb1Only())) {
    return fastEmitInst_r(ARM::tBX_CALL, &ARM::tGPRRegClass, Op0);
  }
  if ((!Subtarget->isThumb()) && (!Subtarget->hasV4TOps())) {
    return fastEmitInst_r(ARM::BMOVPCRX_CALL, &ARM::tGPRRegClass, Op0);
  }
  if ((Subtarget->hasV4TOps()) && (!Subtarget->isThumb())) {
    return fastEmitInst_r(ARM::BX_CALL, &ARM::tGPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_CALL_NOLINK_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_CALL_NOLINK_MVT_i32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::CALL_PRED.

unsigned fastEmit_ARMISD_CALL_PRED_MVT_i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->hasV5TOps()) && (!Subtarget->isThumb()) && ( MF->getSubtarget<ARMSubtarget>().hardenSlsBlr() )) {
    return fastEmitInst_r(ARM::BLX_pred_noip, &ARM::GPRnoipRegClass, Op0);
  }
  if ((Subtarget->hasV5TOps()) && (!Subtarget->isThumb()) && ( !MF->getSubtarget<ARMSubtarget>().hardenSlsBlr() )) {
    return fastEmitInst_r(ARM::BLX_pred, &ARM::GPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_CALL_PRED_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_CALL_PRED_MVT_i32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::CMPFPEw0.

unsigned fastEmit_ARMISD_CMPFPEw0_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(ARM::VCMPEZH, &ARM::HPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_CMPFPEw0_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->hasVFP2Base())) {
    return fastEmitInst_r(ARM::VCMPEZS, &ARM::SPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_CMPFPEw0_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->hasFP64()) && (Subtarget->hasVFP2Base())) {
    return fastEmitInst_r(ARM::VCMPEZD, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_CMPFPEw0_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ARMISD_CMPFPEw0_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ARMISD_CMPFPEw0_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ARMISD_CMPFPEw0_MVT_f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::CMPFPw0.

unsigned fastEmit_ARMISD_CMPFPw0_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(ARM::VCMPZH, &ARM::HPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_CMPFPw0_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->hasVFP2Base())) {
    return fastEmitInst_r(ARM::VCMPZS, &ARM::SPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_CMPFPw0_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->hasFP64()) && (Subtarget->hasVFP2Base())) {
    return fastEmitInst_r(ARM::VCMPZD, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_CMPFPw0_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ARMISD_CMPFPw0_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ARMISD_CMPFPw0_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ARMISD_CMPFPw0_MVT_f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::RRX.

unsigned fastEmit_ARMISD_RRX_MVT_i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb2())) {
    return fastEmitInst_r(ARM::t2RRX, &ARM::rGPRRegClass, Op0);
  }
  if ((!Subtarget->isThumb())) {
    return fastEmitInst_r(ARM::RRX, &ARM::GPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_RRX_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_RRX_MVT_i32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::SRA_FLAG.

unsigned fastEmit_ARMISD_SRA_FLAG_MVT_i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb2())) {
    return fastEmitInst_r(ARM::t2MOVsra_flag, &ARM::rGPRRegClass, Op0);
  }
  if ((!Subtarget->isThumb())) {
    return fastEmitInst_r(ARM::MOVsra_flag, &ARM::GPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_SRA_FLAG_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_SRA_FLAG_MVT_i32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::SRL_FLAG.

unsigned fastEmit_ARMISD_SRL_FLAG_MVT_i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb2())) {
    return fastEmitInst_r(ARM::t2MOVsrl_flag, &ARM::rGPRRegClass, Op0);
  }
  if ((!Subtarget->isThumb())) {
    return fastEmitInst_r(ARM::MOVsrl_flag, &ARM::GPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_SRL_FLAG_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_SRL_FLAG_MVT_i32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VADDVs.

unsigned fastEmit_ARMISD_VADDVs_MVT_v16i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VADDVs8no_acc, &ARM::tGPREvenRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VADDVs_MVT_v8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VADDVs16no_acc, &ARM::tGPREvenRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VADDVs_MVT_v4i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VADDVs32no_acc, &ARM::tGPREvenRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VADDVs_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ARMISD_VADDVs_MVT_v16i8_r(RetVT, Op0);
  case MVT::v8i16: return fastEmit_ARMISD_VADDVs_MVT_v8i16_r(RetVT, Op0);
  case MVT::v4i32: return fastEmit_ARMISD_VADDVs_MVT_v4i32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VADDVu.

unsigned fastEmit_ARMISD_VADDVu_MVT_v16i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VADDVu8no_acc, &ARM::tGPREvenRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VADDVu_MVT_v8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VADDVu16no_acc, &ARM::tGPREvenRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VADDVu_MVT_v4i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VADDVu32no_acc, &ARM::tGPREvenRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VADDVu_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ARMISD_VADDVu_MVT_v16i8_r(RetVT, Op0);
  case MVT::v8i16: return fastEmit_ARMISD_VADDVu_MVT_v8i16_r(RetVT, Op0);
  case MVT::v4i32: return fastEmit_ARMISD_VADDVu_MVT_v4i32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VDUP.

unsigned fastEmit_ARMISD_VDUP_MVT_i32_MVT_v8i8_r(unsigned Op0) {
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VDUP8d, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VDUP_MVT_i32_MVT_v16i8_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VDUP8, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VDUP8q, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VDUP_MVT_i32_MVT_v4i16_r(unsigned Op0) {
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VDUP16d, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VDUP_MVT_i32_MVT_v8i16_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VDUP16, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VDUP16q, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VDUP_MVT_i32_MVT_v2i32_r(unsigned Op0) {
  if ((!Subtarget->hasSlowVDUP32()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VDUP32d, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VDUP_MVT_i32_MVT_v4i32_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VDUP32, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VDUP32q, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VDUP_MVT_i32_MVT_v8f16_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VDUP16, &ARM::MQPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VDUP_MVT_i32_MVT_v4f32_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VDUP32, &ARM::MQPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VDUP_MVT_i32_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ARMISD_VDUP_MVT_i32_MVT_v8i8_r(Op0);
  case MVT::v16i8: return fastEmit_ARMISD_VDUP_MVT_i32_MVT_v16i8_r(Op0);
  case MVT::v4i16: return fastEmit_ARMISD_VDUP_MVT_i32_MVT_v4i16_r(Op0);
  case MVT::v8i16: return fastEmit_ARMISD_VDUP_MVT_i32_MVT_v8i16_r(Op0);
  case MVT::v2i32: return fastEmit_ARMISD_VDUP_MVT_i32_MVT_v2i32_r(Op0);
  case MVT::v4i32: return fastEmit_ARMISD_VDUP_MVT_i32_MVT_v4i32_r(Op0);
  case MVT::v8f16: return fastEmit_ARMISD_VDUP_MVT_i32_MVT_v8f16_r(Op0);
  case MVT::v4f32: return fastEmit_ARMISD_VDUP_MVT_i32_MVT_v4f32_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ARMISD_VDUP_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_VDUP_MVT_i32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VMOVSR.

unsigned fastEmit_ARMISD_VMOVSR_MVT_i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasVFP2Base()) && (Subtarget->preferVMOVSR() ||!Subtarget->useNEONForSinglePrecisionFP())) {
    return fastEmitInst_r(ARM::VMOVSR, &ARM::SPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VMOVSR_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_VMOVSR_MVT_i32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VMOVhr.

unsigned fastEmit_ARMISD_VMOVhr_MVT_i32_MVT_bf16_r(unsigned Op0) {
  if ((Subtarget->hasFPRegs16())) {
    return fastEmitInst_r(ARM::VMOVHR, &ARM::HPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VMOVhr_MVT_i32_MVT_f16_r(unsigned Op0) {
  if ((Subtarget->hasFPRegs16())) {
    return fastEmitInst_r(ARM::VMOVHR, &ARM::HPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VMOVhr_MVT_i32_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::bf16: return fastEmit_ARMISD_VMOVhr_MVT_i32_MVT_bf16_r(Op0);
  case MVT::f16: return fastEmit_ARMISD_VMOVhr_MVT_i32_MVT_f16_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ARMISD_VMOVhr_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_VMOVhr_MVT_i32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VMOVrh.

unsigned fastEmit_ARMISD_VMOVrh_MVT_bf16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasFPRegs16())) {
    return fastEmitInst_r(ARM::VMOVRH, &ARM::rGPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VMOVrh_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasFPRegs16())) {
    return fastEmitInst_r(ARM::VMOVRH, &ARM::rGPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VMOVrh_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::bf16: return fastEmit_ARMISD_VMOVrh_MVT_bf16_r(RetVT, Op0);
  case MVT::f16: return fastEmit_ARMISD_VMOVrh_MVT_f16_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VREV16.

unsigned fastEmit_ARMISD_VREV16_MVT_v8i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VREV16d8, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VREV16_MVT_v16i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VREV16_8, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VREV16q8, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VREV16_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ARMISD_VREV16_MVT_v8i8_r(RetVT, Op0);
  case MVT::v16i8: return fastEmit_ARMISD_VREV16_MVT_v16i8_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VREV32.

unsigned fastEmit_ARMISD_VREV32_MVT_v8i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VREV32d8, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VREV32_MVT_v16i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VREV32_8, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VREV32q8, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VREV32_MVT_v4i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VREV32d16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VREV32_MVT_v8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VREV32_16, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VREV32q16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VREV32_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VREV32d16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VREV32_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VREV32_16, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VREV32q16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VREV32_MVT_v4bf16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4bf16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VREV32d16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VREV32_MVT_v8bf16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8bf16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VREV32q16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VREV32_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ARMISD_VREV32_MVT_v8i8_r(RetVT, Op0);
  case MVT::v16i8: return fastEmit_ARMISD_VREV32_MVT_v16i8_r(RetVT, Op0);
  case MVT::v4i16: return fastEmit_ARMISD_VREV32_MVT_v4i16_r(RetVT, Op0);
  case MVT::v8i16: return fastEmit_ARMISD_VREV32_MVT_v8i16_r(RetVT, Op0);
  case MVT::v4f16: return fastEmit_ARMISD_VREV32_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ARMISD_VREV32_MVT_v8f16_r(RetVT, Op0);
  case MVT::v4bf16: return fastEmit_ARMISD_VREV32_MVT_v4bf16_r(RetVT, Op0);
  case MVT::v8bf16: return fastEmit_ARMISD_VREV32_MVT_v8bf16_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VREV64.

unsigned fastEmit_ARMISD_VREV64_MVT_v8i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VREV64d8, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VREV64_MVT_v16i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VREV64_8, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VREV64q8, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VREV64_MVT_v4i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VREV64d16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VREV64_MVT_v8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VREV64_16, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VREV64q16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VREV64_MVT_v2i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VREV64d32, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VREV64_MVT_v4i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VREV64_32, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VREV64q32, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VREV64_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VREV64d16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VREV64_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VREV64_16, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VREV64q16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VREV64_MVT_v4bf16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4bf16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VREV64d16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VREV64_MVT_v8bf16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8bf16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VREV64q16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VREV64_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VREV64d32, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VREV64_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VREV64_32, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VREV64q32, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VREV64_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ARMISD_VREV64_MVT_v8i8_r(RetVT, Op0);
  case MVT::v16i8: return fastEmit_ARMISD_VREV64_MVT_v16i8_r(RetVT, Op0);
  case MVT::v4i16: return fastEmit_ARMISD_VREV64_MVT_v4i16_r(RetVT, Op0);
  case MVT::v8i16: return fastEmit_ARMISD_VREV64_MVT_v8i16_r(RetVT, Op0);
  case MVT::v2i32: return fastEmit_ARMISD_VREV64_MVT_v2i32_r(RetVT, Op0);
  case MVT::v4i32: return fastEmit_ARMISD_VREV64_MVT_v4i32_r(RetVT, Op0);
  case MVT::v4f16: return fastEmit_ARMISD_VREV64_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ARMISD_VREV64_MVT_v8f16_r(RetVT, Op0);
  case MVT::v4bf16: return fastEmit_ARMISD_VREV64_MVT_v4bf16_r(RetVT, Op0);
  case MVT::v8bf16: return fastEmit_ARMISD_VREV64_MVT_v8bf16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_ARMISD_VREV64_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ARMISD_VREV64_MVT_v4f32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::WIN__DBZCHK.

unsigned fastEmit_ARMISD_WIN__DBZCHK_MVT_i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  return fastEmitInst_r(ARM::WIN__DBZCHK, &ARM::tGPRRegClass, Op0);
}

unsigned fastEmit_ARMISD_WIN__DBZCHK_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_WIN__DBZCHK_MVT_i32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::tSECALL.

unsigned fastEmit_ARMISD_tSECALL_MVT_i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->has8MSecExt()) && (Subtarget->isThumb())) {
    return fastEmitInst_r(ARM::tBLXNS_CALL, &ARM::GPRnopcRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ARMISD_tSECALL_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_tSECALL_MVT_i32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::ABS.

unsigned fastEmit_ISD_ABS_MVT_v8i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VABSv8i8, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_ABS_MVT_v16i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VABSs8, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VABSv16i8, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_ABS_MVT_v4i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VABSv4i16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_ABS_MVT_v8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VABSs16, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VABSv8i16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_ABS_MVT_v2i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VABSv2i32, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_ABS_MVT_v4i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VABSs32, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VABSv4i32, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_ABS_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ISD_ABS_MVT_v8i8_r(RetVT, Op0);
  case MVT::v16i8: return fastEmit_ISD_ABS_MVT_v16i8_r(RetVT, Op0);
  case MVT::v4i16: return fastEmit_ISD_ABS_MVT_v4i16_r(RetVT, Op0);
  case MVT::v8i16: return fastEmit_ISD_ABS_MVT_v8i16_r(RetVT, Op0);
  case MVT::v2i32: return fastEmit_ISD_ABS_MVT_v2i32_r(RetVT, Op0);
  case MVT::v4i32: return fastEmit_ISD_ABS_MVT_v4i32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::ANY_EXTEND.

unsigned fastEmit_ISD_ANY_EXTEND_MVT_v8i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VMOVLuv8i16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_ANY_EXTEND_MVT_v4i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VMOVLuv4i32, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_ANY_EXTEND_MVT_v2i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VMOVLuv2i64, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_ANY_EXTEND_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ISD_ANY_EXTEND_MVT_v8i8_r(RetVT, Op0);
  case MVT::v4i16: return fastEmit_ISD_ANY_EXTEND_MVT_v4i16_r(RetVT, Op0);
  case MVT::v2i32: return fastEmit_ISD_ANY_EXTEND_MVT_v2i32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::BITCAST.

unsigned fastEmit_ISD_BITCAST_MVT_i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPRegs()) && (Subtarget->preferVMOVSR() ||!Subtarget->useNEONForSinglePrecisionFP())) {
    return fastEmitInst_r(ARM::VMOVSR, &ARM::SPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasFPRegs())) {
    return fastEmitInst_r(ARM::VMOVRS, &ARM::GPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_f64_MVT_v8i8_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64d8, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_f64_MVT_v4i16_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64d16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_f64_MVT_v2i32_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64d32, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_f64_MVT_v4f16_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64d16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_f64_MVT_v4bf16_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64d16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_f64_MVT_v2f32_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64d32, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_f64_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ISD_BITCAST_MVT_f64_MVT_v8i8_r(Op0);
  case MVT::v4i16: return fastEmit_ISD_BITCAST_MVT_f64_MVT_v4i16_r(Op0);
  case MVT::v2i32: return fastEmit_ISD_BITCAST_MVT_f64_MVT_v2i32_r(Op0);
  case MVT::v4f16: return fastEmit_ISD_BITCAST_MVT_f64_MVT_v4f16_r(Op0);
  case MVT::v4bf16: return fastEmit_ISD_BITCAST_MVT_f64_MVT_v4bf16_r(Op0);
  case MVT::v2f32: return fastEmit_ISD_BITCAST_MVT_f64_MVT_v2f32_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_BITCAST_MVT_v8i8_MVT_f64_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64d8, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8i8_MVT_v4i16_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV16d8, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8i8_MVT_v2i32_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32d8, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8i8_MVT_v1i64_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64d8, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8i8_MVT_v4f16_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV16d8, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8i8_MVT_v4bf16_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV16d8, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8i8_MVT_v2f32_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32d8, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8i8_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::f64: return fastEmit_ISD_BITCAST_MVT_v8i8_MVT_f64_r(Op0);
  case MVT::v4i16: return fastEmit_ISD_BITCAST_MVT_v8i8_MVT_v4i16_r(Op0);
  case MVT::v2i32: return fastEmit_ISD_BITCAST_MVT_v8i8_MVT_v2i32_r(Op0);
  case MVT::v1i64: return fastEmit_ISD_BITCAST_MVT_v8i8_MVT_v1i64_r(Op0);
  case MVT::v4f16: return fastEmit_ISD_BITCAST_MVT_v8i8_MVT_v4f16_r(Op0);
  case MVT::v4bf16: return fastEmit_ISD_BITCAST_MVT_v8i8_MVT_v4bf16_r(Op0);
  case MVT::v2f32: return fastEmit_ISD_BITCAST_MVT_v8i8_MVT_v2f32_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_BITCAST_MVT_v16i8_MVT_v8i16_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV16_8, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV16q8, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v16i8_MVT_v4i32_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV32_8, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32q8, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v16i8_MVT_v2i64_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV64_8, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64q8, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v16i8_MVT_v8f16_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV16_8, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV16q8, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v16i8_MVT_v8bf16_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV16q8, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v16i8_MVT_v4f32_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV32_8, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32q8, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v16i8_MVT_v2f64_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV64_8, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64q8, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v16i8_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::v8i16: return fastEmit_ISD_BITCAST_MVT_v16i8_MVT_v8i16_r(Op0);
  case MVT::v4i32: return fastEmit_ISD_BITCAST_MVT_v16i8_MVT_v4i32_r(Op0);
  case MVT::v2i64: return fastEmit_ISD_BITCAST_MVT_v16i8_MVT_v2i64_r(Op0);
  case MVT::v8f16: return fastEmit_ISD_BITCAST_MVT_v16i8_MVT_v8f16_r(Op0);
  case MVT::v8bf16: return fastEmit_ISD_BITCAST_MVT_v16i8_MVT_v8bf16_r(Op0);
  case MVT::v4f32: return fastEmit_ISD_BITCAST_MVT_v16i8_MVT_v4f32_r(Op0);
  case MVT::v2f64: return fastEmit_ISD_BITCAST_MVT_v16i8_MVT_v2f64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_BITCAST_MVT_v4i16_MVT_f64_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64d16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4i16_MVT_v8i8_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV16d8, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4i16_MVT_v2i32_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32d16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4i16_MVT_v1i64_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64d16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4i16_MVT_v2f32_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32d16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4i16_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::f64: return fastEmit_ISD_BITCAST_MVT_v4i16_MVT_f64_r(Op0);
  case MVT::v8i8: return fastEmit_ISD_BITCAST_MVT_v4i16_MVT_v8i8_r(Op0);
  case MVT::v2i32: return fastEmit_ISD_BITCAST_MVT_v4i16_MVT_v2i32_r(Op0);
  case MVT::v1i64: return fastEmit_ISD_BITCAST_MVT_v4i16_MVT_v1i64_r(Op0);
  case MVT::v2f32: return fastEmit_ISD_BITCAST_MVT_v4i16_MVT_v2f32_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_BITCAST_MVT_v8i16_MVT_v16i8_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV16_8, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV16q8, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8i16_MVT_v4i32_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV32_16, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32q16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8i16_MVT_v2i64_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV64_16, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64q16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8i16_MVT_v4f32_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV32_16, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32q16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8i16_MVT_v2f64_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV64_16, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64q16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8i16_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_BITCAST_MVT_v8i16_MVT_v16i8_r(Op0);
  case MVT::v4i32: return fastEmit_ISD_BITCAST_MVT_v8i16_MVT_v4i32_r(Op0);
  case MVT::v2i64: return fastEmit_ISD_BITCAST_MVT_v8i16_MVT_v2i64_r(Op0);
  case MVT::v4f32: return fastEmit_ISD_BITCAST_MVT_v8i16_MVT_v4f32_r(Op0);
  case MVT::v2f64: return fastEmit_ISD_BITCAST_MVT_v8i16_MVT_v2f64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_BITCAST_MVT_v2i32_MVT_f64_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64d32, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2i32_MVT_v8i8_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32d8, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2i32_MVT_v4i16_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32d16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2i32_MVT_v1i64_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64d32, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2i32_MVT_v4f16_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32d16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2i32_MVT_v4bf16_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32d16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2i32_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::f64: return fastEmit_ISD_BITCAST_MVT_v2i32_MVT_f64_r(Op0);
  case MVT::v8i8: return fastEmit_ISD_BITCAST_MVT_v2i32_MVT_v8i8_r(Op0);
  case MVT::v4i16: return fastEmit_ISD_BITCAST_MVT_v2i32_MVT_v4i16_r(Op0);
  case MVT::v1i64: return fastEmit_ISD_BITCAST_MVT_v2i32_MVT_v1i64_r(Op0);
  case MVT::v4f16: return fastEmit_ISD_BITCAST_MVT_v2i32_MVT_v4f16_r(Op0);
  case MVT::v4bf16: return fastEmit_ISD_BITCAST_MVT_v2i32_MVT_v4bf16_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_BITCAST_MVT_v4i32_MVT_v16i8_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV32_8, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32q8, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4i32_MVT_v8i16_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV32_16, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32q16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4i32_MVT_v2i64_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV64_32, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64q32, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4i32_MVT_v8f16_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV32_16, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32q16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4i32_MVT_v8bf16_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32q16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4i32_MVT_v2f64_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV64_32, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64q32, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4i32_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_BITCAST_MVT_v4i32_MVT_v16i8_r(Op0);
  case MVT::v8i16: return fastEmit_ISD_BITCAST_MVT_v4i32_MVT_v8i16_r(Op0);
  case MVT::v2i64: return fastEmit_ISD_BITCAST_MVT_v4i32_MVT_v2i64_r(Op0);
  case MVT::v8f16: return fastEmit_ISD_BITCAST_MVT_v4i32_MVT_v8f16_r(Op0);
  case MVT::v8bf16: return fastEmit_ISD_BITCAST_MVT_v4i32_MVT_v8bf16_r(Op0);
  case MVT::v2f64: return fastEmit_ISD_BITCAST_MVT_v4i32_MVT_v2f64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_BITCAST_MVT_v1i64_MVT_v8i8_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64d8, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v1i64_MVT_v4i16_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64d16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v1i64_MVT_v2i32_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64d32, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v1i64_MVT_v4f16_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64d16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v1i64_MVT_v4bf16_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64d16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v1i64_MVT_v2f32_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64d32, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v1i64_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ISD_BITCAST_MVT_v1i64_MVT_v8i8_r(Op0);
  case MVT::v4i16: return fastEmit_ISD_BITCAST_MVT_v1i64_MVT_v4i16_r(Op0);
  case MVT::v2i32: return fastEmit_ISD_BITCAST_MVT_v1i64_MVT_v2i32_r(Op0);
  case MVT::v4f16: return fastEmit_ISD_BITCAST_MVT_v1i64_MVT_v4f16_r(Op0);
  case MVT::v4bf16: return fastEmit_ISD_BITCAST_MVT_v1i64_MVT_v4bf16_r(Op0);
  case MVT::v2f32: return fastEmit_ISD_BITCAST_MVT_v1i64_MVT_v2f32_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_BITCAST_MVT_v2i64_MVT_v16i8_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV64_8, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64q8, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2i64_MVT_v8i16_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV64_16, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64q16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2i64_MVT_v4i32_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV64_32, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64q32, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2i64_MVT_v8f16_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV64_16, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64q16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2i64_MVT_v8bf16_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64q16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2i64_MVT_v4f32_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV64_32, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64q32, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2i64_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_BITCAST_MVT_v2i64_MVT_v16i8_r(Op0);
  case MVT::v8i16: return fastEmit_ISD_BITCAST_MVT_v2i64_MVT_v8i16_r(Op0);
  case MVT::v4i32: return fastEmit_ISD_BITCAST_MVT_v2i64_MVT_v4i32_r(Op0);
  case MVT::v8f16: return fastEmit_ISD_BITCAST_MVT_v2i64_MVT_v8f16_r(Op0);
  case MVT::v8bf16: return fastEmit_ISD_BITCAST_MVT_v2i64_MVT_v8bf16_r(Op0);
  case MVT::v4f32: return fastEmit_ISD_BITCAST_MVT_v2i64_MVT_v4f32_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_BITCAST_MVT_v4f16_MVT_f64_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64d16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4f16_MVT_v8i8_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV16d8, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4f16_MVT_v2i32_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32d16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4f16_MVT_v1i64_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64d16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4f16_MVT_v2f32_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32d16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::f64: return fastEmit_ISD_BITCAST_MVT_v4f16_MVT_f64_r(Op0);
  case MVT::v8i8: return fastEmit_ISD_BITCAST_MVT_v4f16_MVT_v8i8_r(Op0);
  case MVT::v2i32: return fastEmit_ISD_BITCAST_MVT_v4f16_MVT_v2i32_r(Op0);
  case MVT::v1i64: return fastEmit_ISD_BITCAST_MVT_v4f16_MVT_v1i64_r(Op0);
  case MVT::v2f32: return fastEmit_ISD_BITCAST_MVT_v4f16_MVT_v2f32_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_BITCAST_MVT_v8f16_MVT_v16i8_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV16_8, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV16q8, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8f16_MVT_v4i32_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV32_16, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32q16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8f16_MVT_v2i64_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV64_16, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64q16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8f16_MVT_v4f32_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV32_16, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32q16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8f16_MVT_v2f64_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV64_16, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64q16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_BITCAST_MVT_v8f16_MVT_v16i8_r(Op0);
  case MVT::v4i32: return fastEmit_ISD_BITCAST_MVT_v8f16_MVT_v4i32_r(Op0);
  case MVT::v2i64: return fastEmit_ISD_BITCAST_MVT_v8f16_MVT_v2i64_r(Op0);
  case MVT::v4f32: return fastEmit_ISD_BITCAST_MVT_v8f16_MVT_v4f32_r(Op0);
  case MVT::v2f64: return fastEmit_ISD_BITCAST_MVT_v8f16_MVT_v2f64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_BITCAST_MVT_v4bf16_MVT_f64_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64d16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4bf16_MVT_v8i8_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV16d8, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4bf16_MVT_v2i32_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32d16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4bf16_MVT_v1i64_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64d16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4bf16_MVT_v2f32_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32d16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4bf16_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::f64: return fastEmit_ISD_BITCAST_MVT_v4bf16_MVT_f64_r(Op0);
  case MVT::v8i8: return fastEmit_ISD_BITCAST_MVT_v4bf16_MVT_v8i8_r(Op0);
  case MVT::v2i32: return fastEmit_ISD_BITCAST_MVT_v4bf16_MVT_v2i32_r(Op0);
  case MVT::v1i64: return fastEmit_ISD_BITCAST_MVT_v4bf16_MVT_v1i64_r(Op0);
  case MVT::v2f32: return fastEmit_ISD_BITCAST_MVT_v4bf16_MVT_v2f32_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_BITCAST_MVT_v8bf16_MVT_v16i8_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV16q8, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8bf16_MVT_v4i32_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32q16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8bf16_MVT_v2i64_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64q16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8bf16_MVT_v4f32_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32q16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8bf16_MVT_v2f64_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64q16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8bf16_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_BITCAST_MVT_v8bf16_MVT_v16i8_r(Op0);
  case MVT::v4i32: return fastEmit_ISD_BITCAST_MVT_v8bf16_MVT_v4i32_r(Op0);
  case MVT::v2i64: return fastEmit_ISD_BITCAST_MVT_v8bf16_MVT_v2i64_r(Op0);
  case MVT::v4f32: return fastEmit_ISD_BITCAST_MVT_v8bf16_MVT_v4f32_r(Op0);
  case MVT::v2f64: return fastEmit_ISD_BITCAST_MVT_v8bf16_MVT_v2f64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_BITCAST_MVT_v2f32_MVT_f64_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64d32, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2f32_MVT_v8i8_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32d8, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2f32_MVT_v4i16_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32d16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2f32_MVT_v1i64_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64d32, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2f32_MVT_v4f16_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32d16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2f32_MVT_v4bf16_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32d16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::f64: return fastEmit_ISD_BITCAST_MVT_v2f32_MVT_f64_r(Op0);
  case MVT::v8i8: return fastEmit_ISD_BITCAST_MVT_v2f32_MVT_v8i8_r(Op0);
  case MVT::v4i16: return fastEmit_ISD_BITCAST_MVT_v2f32_MVT_v4i16_r(Op0);
  case MVT::v1i64: return fastEmit_ISD_BITCAST_MVT_v2f32_MVT_v1i64_r(Op0);
  case MVT::v4f16: return fastEmit_ISD_BITCAST_MVT_v2f32_MVT_v4f16_r(Op0);
  case MVT::v4bf16: return fastEmit_ISD_BITCAST_MVT_v2f32_MVT_v4bf16_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_BITCAST_MVT_v4f32_MVT_v16i8_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV32_8, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32q8, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4f32_MVT_v8i16_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV32_16, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32q16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4f32_MVT_v2i64_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV64_32, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64q32, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4f32_MVT_v8f16_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV32_16, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32q16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4f32_MVT_v8bf16_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV32q16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4f32_MVT_v2f64_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV64_32, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64q32, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_BITCAST_MVT_v4f32_MVT_v16i8_r(Op0);
  case MVT::v8i16: return fastEmit_ISD_BITCAST_MVT_v4f32_MVT_v8i16_r(Op0);
  case MVT::v2i64: return fastEmit_ISD_BITCAST_MVT_v4f32_MVT_v2i64_r(Op0);
  case MVT::v8f16: return fastEmit_ISD_BITCAST_MVT_v4f32_MVT_v8f16_r(Op0);
  case MVT::v8bf16: return fastEmit_ISD_BITCAST_MVT_v4f32_MVT_v8bf16_r(Op0);
  case MVT::v2f64: return fastEmit_ISD_BITCAST_MVT_v4f32_MVT_v2f64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_BITCAST_MVT_v2f64_MVT_v16i8_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV64_8, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64q8, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2f64_MVT_v8i16_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV64_16, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64q16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2f64_MVT_v4i32_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV64_32, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64q32, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2f64_MVT_v8f16_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV64_16, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64q16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2f64_MVT_v8bf16_r(unsigned Op0) {
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64q16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2f64_MVT_v4f32_r(unsigned Op0) {
  if ((Subtarget->hasMVEIntegerOps()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::MVE_VREV64_32, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON()) && (MF->getDataLayout().isBigEndian())) {
    return fastEmitInst_r(ARM::VREV64q32, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_BITCAST_MVT_v2f64_MVT_v16i8_r(Op0);
  case MVT::v8i16: return fastEmit_ISD_BITCAST_MVT_v2f64_MVT_v8i16_r(Op0);
  case MVT::v4i32: return fastEmit_ISD_BITCAST_MVT_v2f64_MVT_v4i32_r(Op0);
  case MVT::v8f16: return fastEmit_ISD_BITCAST_MVT_v2f64_MVT_v8f16_r(Op0);
  case MVT::v8bf16: return fastEmit_ISD_BITCAST_MVT_v2f64_MVT_v8bf16_r(Op0);
  case MVT::v4f32: return fastEmit_ISD_BITCAST_MVT_v2f64_MVT_v4f32_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_BITCAST_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_BITCAST_MVT_i32_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_BITCAST_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_BITCAST_MVT_f64_r(RetVT, Op0);
  case MVT::v8i8: return fastEmit_ISD_BITCAST_MVT_v8i8_r(RetVT, Op0);
  case MVT::v16i8: return fastEmit_ISD_BITCAST_MVT_v16i8_r(RetVT, Op0);
  case MVT::v4i16: return fastEmit_ISD_BITCAST_MVT_v4i16_r(RetVT, Op0);
  case MVT::v8i16: return fastEmit_ISD_BITCAST_MVT_v8i16_r(RetVT, Op0);
  case MVT::v2i32: return fastEmit_ISD_BITCAST_MVT_v2i32_r(RetVT, Op0);
  case MVT::v4i32: return fastEmit_ISD_BITCAST_MVT_v4i32_r(RetVT, Op0);
  case MVT::v1i64: return fastEmit_ISD_BITCAST_MVT_v1i64_r(RetVT, Op0);
  case MVT::v2i64: return fastEmit_ISD_BITCAST_MVT_v2i64_r(RetVT, Op0);
  case MVT::v4f16: return fastEmit_ISD_BITCAST_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ISD_BITCAST_MVT_v8f16_r(RetVT, Op0);
  case MVT::v4bf16: return fastEmit_ISD_BITCAST_MVT_v4bf16_r(RetVT, Op0);
  case MVT::v8bf16: return fastEmit_ISD_BITCAST_MVT_v8bf16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_ISD_BITCAST_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_BITCAST_MVT_v4f32_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_ISD_BITCAST_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::BITREVERSE.

unsigned fastEmit_ISD_BITREVERSE_MVT_i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb2())) {
    return fastEmitInst_r(ARM::t2RBIT, &ARM::rGPRRegClass, Op0);
  }
  if ((Subtarget->hasV6T2Ops()) && (!Subtarget->isThumb())) {
    return fastEmitInst_r(ARM::RBIT, &ARM::GPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITREVERSE_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_BITREVERSE_MVT_i32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::BRIND.

unsigned fastEmit_ISD_BRIND_MVT_i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->isThumb())) {
    return fastEmitInst_r(ARM::tBRIND, &ARM::GPRRegClass, Op0);
  }
  if ((!Subtarget->isThumb()) && (!Subtarget->hasV4TOps())) {
    return fastEmitInst_r(ARM::MOVPCRX, &ARM::GPRRegClass, Op0);
  }
  if ((Subtarget->hasV4TOps()) && (!Subtarget->isThumb())) {
    return fastEmitInst_r(ARM::BX, &ARM::GPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BRIND_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_BRIND_MVT_i32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::BSWAP.

unsigned fastEmit_ISD_BSWAP_MVT_i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb2())) {
    return fastEmitInst_r(ARM::t2REV, &ARM::rGPRRegClass, Op0);
  }
  if ((Subtarget->hasV6Ops()) && (Subtarget->isThumb()) && (Subtarget->isThumb1Only())) {
    return fastEmitInst_r(ARM::tREV, &ARM::tGPRRegClass, Op0);
  }
  if ((Subtarget->hasV6Ops()) && (!Subtarget->isThumb())) {
    return fastEmitInst_r(ARM::REV, &ARM::GPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BSWAP_MVT_v8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VREV16_8, &ARM::MQPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BSWAP_MVT_v4i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VREV32_8, &ARM::MQPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BSWAP_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_BSWAP_MVT_i32_r(RetVT, Op0);
  case MVT::v8i16: return fastEmit_ISD_BSWAP_MVT_v8i16_r(RetVT, Op0);
  case MVT::v4i32: return fastEmit_ISD_BSWAP_MVT_v4i32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::CTLZ.

unsigned fastEmit_ISD_CTLZ_MVT_i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb2())) {
    return fastEmitInst_r(ARM::t2CLZ, &ARM::rGPRRegClass, Op0);
  }
  if ((Subtarget->hasV5TOps()) && (!Subtarget->isThumb())) {
    return fastEmitInst_r(ARM::CLZ, &ARM::GPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_CTLZ_MVT_v8i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VCLZv8i8, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_CTLZ_MVT_v16i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VCLZs8, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VCLZv16i8, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_CTLZ_MVT_v4i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VCLZv4i16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_CTLZ_MVT_v8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VCLZs16, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VCLZv8i16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_CTLZ_MVT_v2i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VCLZv2i32, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_CTLZ_MVT_v4i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VCLZs32, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VCLZv4i32, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_CTLZ_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_CTLZ_MVT_i32_r(RetVT, Op0);
  case MVT::v8i8: return fastEmit_ISD_CTLZ_MVT_v8i8_r(RetVT, Op0);
  case MVT::v16i8: return fastEmit_ISD_CTLZ_MVT_v16i8_r(RetVT, Op0);
  case MVT::v4i16: return fastEmit_ISD_CTLZ_MVT_v4i16_r(RetVT, Op0);
  case MVT::v8i16: return fastEmit_ISD_CTLZ_MVT_v8i16_r(RetVT, Op0);
  case MVT::v2i32: return fastEmit_ISD_CTLZ_MVT_v2i32_r(RetVT, Op0);
  case MVT::v4i32: return fastEmit_ISD_CTLZ_MVT_v4i32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::CTPOP.

unsigned fastEmit_ISD_CTPOP_MVT_v8i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VCNTd, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_CTPOP_MVT_v16i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VCNTq, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_CTPOP_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ISD_CTPOP_MVT_v8i8_r(RetVT, Op0);
  case MVT::v16i8: return fastEmit_ISD_CTPOP_MVT_v16i8_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::FABS.

unsigned fastEmit_ISD_FABS_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(ARM::VABSH, &ARM::HPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FABS_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((!Subtarget->useNEONForSinglePrecisionFP()) && (Subtarget->hasVFP2Base())) {
    return fastEmitInst_r(ARM::VABSS, &ARM::SPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FABS_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFP64()) && (Subtarget->hasVFP2Base())) {
    return fastEmitInst_r(ARM::VABSD, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FABS_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VABShd, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FABS_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VABSf16, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VABShq, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FABS_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VABSfd, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FABS_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VABSf32, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VABSfq, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FABS_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FABS_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_FABS_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_FABS_MVT_f64_r(RetVT, Op0);
  case MVT::v4f16: return fastEmit_ISD_FABS_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ISD_FABS_MVT_v8f16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_ISD_FABS_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_FABS_MVT_v4f32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::FCEIL.

unsigned fastEmit_ISD_FCEIL_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(ARM::VRINTPH, &ARM::HPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FCEIL_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8Base())) {
    return fastEmitInst_r(ARM::VRINTPS, &ARM::SPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FCEIL_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFP64()) && (Subtarget->hasFPARMv8Base())) {
    return fastEmitInst_r(ARM::VRINTPD, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FCEIL_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasMVEFloatOps())) {
    return fastEmitInst_r(ARM::MVE_VRINTf16P, &ARM::MQPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FCEIL_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasMVEFloatOps())) {
    return fastEmitInst_r(ARM::MVE_VRINTf32P, &ARM::MQPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FCEIL_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FCEIL_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_FCEIL_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_FCEIL_MVT_f64_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ISD_FCEIL_MVT_v8f16_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_FCEIL_MVT_v4f32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::FFLOOR.

unsigned fastEmit_ISD_FFLOOR_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(ARM::VRINTMH, &ARM::HPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FFLOOR_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8Base())) {
    return fastEmitInst_r(ARM::VRINTMS, &ARM::SPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FFLOOR_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFP64()) && (Subtarget->hasFPARMv8Base())) {
    return fastEmitInst_r(ARM::VRINTMD, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FFLOOR_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasMVEFloatOps())) {
    return fastEmitInst_r(ARM::MVE_VRINTf16M, &ARM::MQPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FFLOOR_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasMVEFloatOps())) {
    return fastEmitInst_r(ARM::MVE_VRINTf32M, &ARM::MQPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FFLOOR_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FFLOOR_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_FFLOOR_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_FFLOOR_MVT_f64_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ISD_FFLOOR_MVT_v8f16_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_FFLOOR_MVT_v4f32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::FNEARBYINT.

unsigned fastEmit_ISD_FNEARBYINT_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(ARM::VRINTRH, &ARM::HPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEARBYINT_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8Base())) {
    return fastEmitInst_r(ARM::VRINTRS, &ARM::SPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEARBYINT_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFP64()) && (Subtarget->hasFPARMv8Base())) {
    return fastEmitInst_r(ARM::VRINTRD, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEARBYINT_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FNEARBYINT_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_FNEARBYINT_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_FNEARBYINT_MVT_f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::FNEG.

unsigned fastEmit_ISD_FNEG_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(ARM::VNEGH, &ARM::HPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEG_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((!Subtarget->useNEONForSinglePrecisionFP()) && (Subtarget->hasVFP2Base())) {
    return fastEmitInst_r(ARM::VNEGS, &ARM::SPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEG_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFP64()) && (Subtarget->hasVFP2Base())) {
    return fastEmitInst_r(ARM::VNEGD, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEG_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VNEGhd, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEG_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VNEGf16, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VNEGhq, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEG_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VNEGfd, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEG_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VNEGf32, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VNEGf32q, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEG_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FNEG_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_FNEG_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_FNEG_MVT_f64_r(RetVT, Op0);
  case MVT::v4f16: return fastEmit_ISD_FNEG_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ISD_FNEG_MVT_v8f16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_ISD_FNEG_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_FNEG_MVT_v4f32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::FP_EXTEND.

unsigned fastEmit_ISD_FP_EXTEND_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFP64()) && (Subtarget->hasVFP2Base())) {
    return fastEmitInst_r(ARM::VCVTDS, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_EXTEND_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  return fastEmitInst_r(ARM::VCVTh2f, &ARM::QPRRegClass, Op0);
}

unsigned fastEmit_ISD_FP_EXTEND_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FP_EXTEND_MVT_f32_r(RetVT, Op0);
  case MVT::v4f16: return fastEmit_ISD_FP_EXTEND_MVT_v4f16_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::FP_ROUND.

unsigned fastEmit_ISD_FP_ROUND_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFP64()) && (Subtarget->hasVFP2Base())) {
    return fastEmitInst_r(ARM::VCVTSD, &ARM::SPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_ROUND_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  return fastEmitInst_r(ARM::VCVTf2h, &ARM::DPRRegClass, Op0);
}

unsigned fastEmit_ISD_FP_ROUND_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f64: return fastEmit_ISD_FP_ROUND_MVT_f64_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_FP_ROUND_MVT_v4f32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::FP_TO_SINT.

unsigned fastEmit_ISD_FP_TO_SINT_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VCVTh2sd, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_SINT_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMVEFloatOps())) {
    return fastEmitInst_r(ARM::MVE_VCVTs16f16z, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VCVTh2sq, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_SINT_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VCVTf2sd, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_SINT_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMVEFloatOps())) {
    return fastEmitInst_r(ARM::MVE_VCVTs32f32z, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VCVTf2sq, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_SINT_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v4f16: return fastEmit_ISD_FP_TO_SINT_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ISD_FP_TO_SINT_MVT_v8f16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_ISD_FP_TO_SINT_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_FP_TO_SINT_MVT_v4f32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::FP_TO_UINT.

unsigned fastEmit_ISD_FP_TO_UINT_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VCVTh2ud, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_UINT_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMVEFloatOps())) {
    return fastEmitInst_r(ARM::MVE_VCVTu16f16z, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VCVTh2uq, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_UINT_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VCVTf2ud, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_UINT_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMVEFloatOps())) {
    return fastEmitInst_r(ARM::MVE_VCVTu32f32z, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VCVTf2uq, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_UINT_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v4f16: return fastEmit_ISD_FP_TO_UINT_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ISD_FP_TO_UINT_MVT_v8f16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_ISD_FP_TO_UINT_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_FP_TO_UINT_MVT_v4f32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::FRINT.

unsigned fastEmit_ISD_FRINT_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(ARM::VRINTXH, &ARM::HPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FRINT_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8Base())) {
    return fastEmitInst_r(ARM::VRINTXS, &ARM::SPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FRINT_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFP64()) && (Subtarget->hasFPARMv8Base())) {
    return fastEmitInst_r(ARM::VRINTXD, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FRINT_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasMVEFloatOps())) {
    return fastEmitInst_r(ARM::MVE_VRINTf16X, &ARM::MQPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FRINT_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasMVEFloatOps())) {
    return fastEmitInst_r(ARM::MVE_VRINTf32X, &ARM::MQPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FRINT_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FRINT_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_FRINT_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_FRINT_MVT_f64_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ISD_FRINT_MVT_v8f16_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_FRINT_MVT_v4f32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::FROUND.

unsigned fastEmit_ISD_FROUND_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(ARM::VRINTAH, &ARM::HPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FROUND_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8Base())) {
    return fastEmitInst_r(ARM::VRINTAS, &ARM::SPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FROUND_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFP64()) && (Subtarget->hasFPARMv8Base())) {
    return fastEmitInst_r(ARM::VRINTAD, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FROUND_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasMVEFloatOps())) {
    return fastEmitInst_r(ARM::MVE_VRINTf16A, &ARM::MQPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FROUND_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasMVEFloatOps())) {
    return fastEmitInst_r(ARM::MVE_VRINTf32A, &ARM::MQPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FROUND_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FROUND_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_FROUND_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_FROUND_MVT_f64_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ISD_FROUND_MVT_v8f16_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_FROUND_MVT_v4f32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::FSQRT.

unsigned fastEmit_ISD_FSQRT_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(ARM::VSQRTH, &ARM::HPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FSQRT_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasVFP2Base())) {
    return fastEmitInst_r(ARM::VSQRTS, &ARM::SPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FSQRT_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFP64()) && (Subtarget->hasVFP2Base())) {
    return fastEmitInst_r(ARM::VSQRTD, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FSQRT_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FSQRT_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_FSQRT_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_FSQRT_MVT_f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::FTRUNC.

unsigned fastEmit_ISD_FTRUNC_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(ARM::VRINTZH, &ARM::HPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FTRUNC_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8Base())) {
    return fastEmitInst_r(ARM::VRINTZS, &ARM::SPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FTRUNC_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFP64()) && (Subtarget->hasFPARMv8Base())) {
    return fastEmitInst_r(ARM::VRINTZD, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FTRUNC_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasMVEFloatOps())) {
    return fastEmitInst_r(ARM::MVE_VRINTf16Z, &ARM::MQPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FTRUNC_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasMVEFloatOps())) {
    return fastEmitInst_r(ARM::MVE_VRINTf32Z, &ARM::MQPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FTRUNC_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FTRUNC_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_FTRUNC_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_FTRUNC_MVT_f64_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ISD_FTRUNC_MVT_v8f16_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_FTRUNC_MVT_v4f32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::SIGN_EXTEND.

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_v8i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VMOVLsv8i16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_v4i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VMOVLsv4i32, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_v2i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VMOVLsv2i64, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ISD_SIGN_EXTEND_MVT_v8i8_r(RetVT, Op0);
  case MVT::v4i16: return fastEmit_ISD_SIGN_EXTEND_MVT_v4i16_r(RetVT, Op0);
  case MVT::v2i32: return fastEmit_ISD_SIGN_EXTEND_MVT_v2i32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::SINT_TO_FP.

unsigned fastEmit_ISD_SINT_TO_FP_MVT_v4i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VCVTs2hd, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_v8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasMVEFloatOps())) {
    return fastEmitInst_r(ARM::MVE_VCVTf16s16n, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VCVTs2hq, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_v2i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VCVTs2fd, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_v4i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasMVEFloatOps())) {
    return fastEmitInst_r(ARM::MVE_VCVTf32s32n, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VCVTs2fq, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v4i16: return fastEmit_ISD_SINT_TO_FP_MVT_v4i16_r(RetVT, Op0);
  case MVT::v8i16: return fastEmit_ISD_SINT_TO_FP_MVT_v8i16_r(RetVT, Op0);
  case MVT::v2i32: return fastEmit_ISD_SINT_TO_FP_MVT_v2i32_r(RetVT, Op0);
  case MVT::v4i32: return fastEmit_ISD_SINT_TO_FP_MVT_v4i32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::TRUNCATE.

unsigned fastEmit_ISD_TRUNCATE_MVT_v8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VMOVNv8i8, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_TRUNCATE_MVT_v4i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VMOVNv4i16, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_TRUNCATE_MVT_v2i64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VMOVNv2i32, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_TRUNCATE_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_ISD_TRUNCATE_MVT_v8i16_r(RetVT, Op0);
  case MVT::v4i32: return fastEmit_ISD_TRUNCATE_MVT_v4i32_r(RetVT, Op0);
  case MVT::v2i64: return fastEmit_ISD_TRUNCATE_MVT_v2i64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::UINT_TO_FP.

unsigned fastEmit_ISD_UINT_TO_FP_MVT_v4i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VCVTu2hd, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_v8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasMVEFloatOps())) {
    return fastEmitInst_r(ARM::MVE_VCVTf16u16n, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VCVTu2hq, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_v2i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VCVTu2fd, &ARM::DPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_v4i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasMVEFloatOps())) {
    return fastEmitInst_r(ARM::MVE_VCVTf32u32n, &ARM::MQPRRegClass, Op0);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VCVTu2fq, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_UINT_TO_FP_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v4i16: return fastEmit_ISD_UINT_TO_FP_MVT_v4i16_r(RetVT, Op0);
  case MVT::v8i16: return fastEmit_ISD_UINT_TO_FP_MVT_v8i16_r(RetVT, Op0);
  case MVT::v2i32: return fastEmit_ISD_UINT_TO_FP_MVT_v2i32_r(RetVT, Op0);
  case MVT::v4i32: return fastEmit_ISD_UINT_TO_FP_MVT_v4i32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::VECREDUCE_ADD.

unsigned fastEmit_ISD_VECREDUCE_ADD_MVT_v16i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VADDVu8no_acc, &ARM::tGPREvenRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_VECREDUCE_ADD_MVT_v8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VADDVu16no_acc, &ARM::tGPREvenRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_VECREDUCE_ADD_MVT_v4i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_r(ARM::MVE_VADDVu32no_acc, &ARM::tGPREvenRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_VECREDUCE_ADD_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_VECREDUCE_ADD_MVT_v16i8_r(RetVT, Op0);
  case MVT::v8i16: return fastEmit_ISD_VECREDUCE_ADD_MVT_v8i16_r(RetVT, Op0);
  case MVT::v4i32: return fastEmit_ISD_VECREDUCE_ADD_MVT_v4i32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::ZERO_EXTEND.

unsigned fastEmit_ISD_ZERO_EXTEND_MVT_v8i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VMOVLuv8i16, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_ZERO_EXTEND_MVT_v4i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VMOVLuv4i32, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_ZERO_EXTEND_MVT_v2i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(ARM::VMOVLuv2i64, &ARM::QPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_ZERO_EXTEND_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ISD_ZERO_EXTEND_MVT_v8i8_r(RetVT, Op0);
  case MVT::v4i16: return fastEmit_ISD_ZERO_EXTEND_MVT_v4i16_r(RetVT, Op0);
  case MVT::v2i32: return fastEmit_ISD_ZERO_EXTEND_MVT_v2i32_r(RetVT, Op0);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_r(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0) override {
  switch (Opcode) {
  case ARMISD::CALL: return fastEmit_ARMISD_CALL_r(VT, RetVT, Op0);
  case ARMISD::CALL_NOLINK: return fastEmit_ARMISD_CALL_NOLINK_r(VT, RetVT, Op0);
  case ARMISD::CALL_PRED: return fastEmit_ARMISD_CALL_PRED_r(VT, RetVT, Op0);
  case ARMISD::CMPFPEw0: return fastEmit_ARMISD_CMPFPEw0_r(VT, RetVT, Op0);
  case ARMISD::CMPFPw0: return fastEmit_ARMISD_CMPFPw0_r(VT, RetVT, Op0);
  case ARMISD::RRX: return fastEmit_ARMISD_RRX_r(VT, RetVT, Op0);
  case ARMISD::SRA_FLAG: return fastEmit_ARMISD_SRA_FLAG_r(VT, RetVT, Op0);
  case ARMISD::SRL_FLAG: return fastEmit_ARMISD_SRL_FLAG_r(VT, RetVT, Op0);
  case ARMISD::VADDVs: return fastEmit_ARMISD_VADDVs_r(VT, RetVT, Op0);
  case ARMISD::VADDVu: return fastEmit_ARMISD_VADDVu_r(VT, RetVT, Op0);
  case ARMISD::VDUP: return fastEmit_ARMISD_VDUP_r(VT, RetVT, Op0);
  case ARMISD::VMOVSR: return fastEmit_ARMISD_VMOVSR_r(VT, RetVT, Op0);
  case ARMISD::VMOVhr: return fastEmit_ARMISD_VMOVhr_r(VT, RetVT, Op0);
  case ARMISD::VMOVrh: return fastEmit_ARMISD_VMOVrh_r(VT, RetVT, Op0);
  case ARMISD::VREV16: return fastEmit_ARMISD_VREV16_r(VT, RetVT, Op0);
  case ARMISD::VREV32: return fastEmit_ARMISD_VREV32_r(VT, RetVT, Op0);
  case ARMISD::VREV64: return fastEmit_ARMISD_VREV64_r(VT, RetVT, Op0);
  case ARMISD::WIN__DBZCHK: return fastEmit_ARMISD_WIN__DBZCHK_r(VT, RetVT, Op0);
  case ARMISD::tSECALL: return fastEmit_ARMISD_tSECALL_r(VT, RetVT, Op0);
  case ISD::ABS: return fastEmit_ISD_ABS_r(VT, RetVT, Op0);
  case ISD::ANY_EXTEND: return fastEmit_ISD_ANY_EXTEND_r(VT, RetVT, Op0);
  case ISD::BITCAST: return fastEmit_ISD_BITCAST_r(VT, RetVT, Op0);
  case ISD::BITREVERSE: return fastEmit_ISD_BITREVERSE_r(VT, RetVT, Op0);
  case ISD::BRIND: return fastEmit_ISD_BRIND_r(VT, RetVT, Op0);
  case ISD::BSWAP: return fastEmit_ISD_BSWAP_r(VT, RetVT, Op0);
  case ISD::CTLZ: return fastEmit_ISD_CTLZ_r(VT, RetVT, Op0);
  case ISD::CTPOP: return fastEmit_ISD_CTPOP_r(VT, RetVT, Op0);
  case ISD::FABS: return fastEmit_ISD_FABS_r(VT, RetVT, Op0);
  case ISD::FCEIL: return fastEmit_ISD_FCEIL_r(VT, RetVT, Op0);
  case ISD::FFLOOR: return fastEmit_ISD_FFLOOR_r(VT, RetVT, Op0);
  case ISD::FNEARBYINT: return fastEmit_ISD_FNEARBYINT_r(VT, RetVT, Op0);
  case ISD::FNEG: return fastEmit_ISD_FNEG_r(VT, RetVT, Op0);
  case ISD::FP_EXTEND: return fastEmit_ISD_FP_EXTEND_r(VT, RetVT, Op0);
  case ISD::FP_ROUND: return fastEmit_ISD_FP_ROUND_r(VT, RetVT, Op0);
  case ISD::FP_TO_SINT: return fastEmit_ISD_FP_TO_SINT_r(VT, RetVT, Op0);
  case ISD::FP_TO_UINT: return fastEmit_ISD_FP_TO_UINT_r(VT, RetVT, Op0);
  case ISD::FRINT: return fastEmit_ISD_FRINT_r(VT, RetVT, Op0);
  case ISD::FROUND: return fastEmit_ISD_FROUND_r(VT, RetVT, Op0);
  case ISD::FSQRT: return fastEmit_ISD_FSQRT_r(VT, RetVT, Op0);
  case ISD::FTRUNC: return fastEmit_ISD_FTRUNC_r(VT, RetVT, Op0);
  case ISD::SIGN_EXTEND: return fastEmit_ISD_SIGN_EXTEND_r(VT, RetVT, Op0);
  case ISD::SINT_TO_FP: return fastEmit_ISD_SINT_TO_FP_r(VT, RetVT, Op0);
  case ISD::TRUNCATE: return fastEmit_ISD_TRUNCATE_r(VT, RetVT, Op0);
  case ISD::UINT_TO_FP: return fastEmit_ISD_UINT_TO_FP_r(VT, RetVT, Op0);
  case ISD::VECREDUCE_ADD: return fastEmit_ISD_VECREDUCE_ADD_r(VT, RetVT, Op0);
  case ISD::ZERO_EXTEND: return fastEmit_ISD_ZERO_EXTEND_r(VT, RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::CMP.

unsigned fastEmit_ARMISD_CMP_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->isThumb2())) {
    return fastEmitInst_rr(ARM::t2CMPrr, &ARM::GPRnopcRegClass, Op0, Op1);
  }
  if ((Subtarget->isThumb()) && (Subtarget->isThumb1Only())) {
    return fastEmitInst_rr(ARM::tCMPr, &ARM::tGPRRegClass, Op0, Op1);
  }
  if ((!Subtarget->isThumb())) {
    return fastEmitInst_rr(ARM::CMPrr, &ARM::GPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_CMP_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_CMP_MVT_i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::CMPFP.

unsigned fastEmit_ARMISD_CMPFP_MVT_f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_rr(ARM::VCMPH, &ARM::HPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_CMPFP_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->hasVFP2Base())) {
    return fastEmitInst_rr(ARM::VCMPS, &ARM::SPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_CMPFP_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->hasFP64()) && (Subtarget->hasVFP2Base())) {
    return fastEmitInst_rr(ARM::VCMPD, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_CMPFP_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ARMISD_CMPFP_MVT_f16_rr(RetVT, Op0, Op1);
  case MVT::f32: return fastEmit_ARMISD_CMPFP_MVT_f32_rr(RetVT, Op0, Op1);
  case MVT::f64: return fastEmit_ARMISD_CMPFP_MVT_f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::CMPFPE.

unsigned fastEmit_ARMISD_CMPFPE_MVT_f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_rr(ARM::VCMPEH, &ARM::HPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_CMPFPE_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->hasVFP2Base())) {
    return fastEmitInst_rr(ARM::VCMPES, &ARM::SPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_CMPFPE_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->hasFP64()) && (Subtarget->hasVFP2Base())) {
    return fastEmitInst_rr(ARM::VCMPED, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_CMPFPE_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ARMISD_CMPFPE_MVT_f16_rr(RetVT, Op0, Op1);
  case MVT::f32: return fastEmit_ARMISD_CMPFPE_MVT_f32_rr(RetVT, Op0, Op1);
  case MVT::f64: return fastEmit_ARMISD_CMPFPE_MVT_f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::CMPZ.

unsigned fastEmit_ARMISD_CMPZ_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->isThumb2())) {
    return fastEmitInst_rr(ARM::t2CMPrr, &ARM::GPRnopcRegClass, Op0, Op1);
  }
  if ((Subtarget->isThumb()) && (Subtarget->isThumb1Only())) {
    return fastEmitInst_rr(ARM::tCMPr, &ARM::tGPRRegClass, Op0, Op1);
  }
  if ((!Subtarget->isThumb())) {
    return fastEmitInst_rr(ARM::CMPrr, &ARM::GPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_CMPZ_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_CMPZ_MVT_i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::EH_SJLJ_LONGJMP.

unsigned fastEmit_ARMISD_EH_SJLJ_LONGJMP_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->isThumb()) && (Subtarget->isTargetWindows())) {
    return fastEmitInst_rr(ARM::tInt_WIN_eh_sjlj_longjmp, &ARM::GPRRegClass, Op0, Op1);
  }
  if ((!Subtarget->isTargetWindows()) && (Subtarget->isThumb())) {
    return fastEmitInst_rr(ARM::tInt_eh_sjlj_longjmp, &ARM::tGPRRegClass, Op0, Op1);
  }
  if ((!Subtarget->isThumb())) {
    return fastEmitInst_rr(ARM::Int_eh_sjlj_longjmp, &ARM::GPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_EH_SJLJ_LONGJMP_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_EH_SJLJ_LONGJMP_MVT_i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::EH_SJLJ_SETJMP.

unsigned fastEmit_ARMISD_EH_SJLJ_SETJMP_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb2()) && (!Subtarget->hasVFP2Base())) {
    return fastEmitInst_rr(ARM::t2Int_eh_sjlj_setjmp_nofp, &ARM::tGPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasVFP2Base()) && (Subtarget->isThumb2())) {
    return fastEmitInst_rr(ARM::t2Int_eh_sjlj_setjmp, &ARM::tGPRRegClass, Op0, Op1);
  }
  if ((Subtarget->isThumb()) && (Subtarget->isThumb1Only())) {
    return fastEmitInst_rr(ARM::tInt_eh_sjlj_setjmp, &ARM::tGPRRegClass, Op0, Op1);
  }
  if ((!Subtarget->isThumb()) && (!Subtarget->hasVFP2Base())) {
    return fastEmitInst_rr(ARM::Int_eh_sjlj_setjmp_nofp, &ARM::GPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasVFP2Base()) && (!Subtarget->isThumb())) {
    return fastEmitInst_rr(ARM::Int_eh_sjlj_setjmp, &ARM::GPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_EH_SJLJ_SETJMP_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_EH_SJLJ_SETJMP_MVT_i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::QADD16b.

unsigned fastEmit_ARMISD_QADD16b_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasDSP()) && (Subtarget->isThumb2())) {
    return fastEmitInst_rr(ARM::t2QADD16, &ARM::rGPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasV6Ops()) && (!Subtarget->isThumb())) {
    return fastEmitInst_rr(ARM::QADD16, &ARM::GPRnopcRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_QADD16b_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_QADD16b_MVT_i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::QADD8b.

unsigned fastEmit_ARMISD_QADD8b_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasDSP()) && (Subtarget->isThumb2())) {
    return fastEmitInst_rr(ARM::t2QADD8, &ARM::rGPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasV6Ops()) && (!Subtarget->isThumb())) {
    return fastEmitInst_rr(ARM::QADD8, &ARM::GPRnopcRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_QADD8b_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_QADD8b_MVT_i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::QSUB16b.

unsigned fastEmit_ARMISD_QSUB16b_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasDSP()) && (Subtarget->isThumb2())) {
    return fastEmitInst_rr(ARM::t2QSUB16, &ARM::rGPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasV6Ops()) && (!Subtarget->isThumb())) {
    return fastEmitInst_rr(ARM::QSUB16, &ARM::GPRnopcRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_QSUB16b_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_QSUB16b_MVT_i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::QSUB8b.

unsigned fastEmit_ARMISD_QSUB8b_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasDSP()) && (Subtarget->isThumb2())) {
    return fastEmitInst_rr(ARM::t2QSUB8, &ARM::rGPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasV6Ops()) && (!Subtarget->isThumb())) {
    return fastEmitInst_rr(ARM::QSUB8, &ARM::GPRnopcRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_QSUB8b_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_QSUB8b_MVT_i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::SMULWB.

unsigned fastEmit_ARMISD_SMULWB_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasDSP()) && (Subtarget->isThumb2())) {
    return fastEmitInst_rr(ARM::t2SMULWB, &ARM::rGPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasV5TEOps()) && (!Subtarget->isThumb())) {
    return fastEmitInst_rr(ARM::SMULWB, &ARM::GPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_SMULWB_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_SMULWB_MVT_i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::SMULWT.

unsigned fastEmit_ARMISD_SMULWT_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasDSP()) && (Subtarget->isThumb2())) {
    return fastEmitInst_rr(ARM::t2SMULWT, &ARM::rGPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasV5TEOps()) && (!Subtarget->isThumb())) {
    return fastEmitInst_rr(ARM::SMULWT, &ARM::GPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_SMULWT_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_SMULWT_MVT_i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::SUBS.

unsigned fastEmit_ARMISD_SUBS_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb2())) {
    return fastEmitInst_rr(ARM::t2SUBSrr, &ARM::rGPRRegClass, Op0, Op1);
  }
  if ((Subtarget->isThumb()) && (Subtarget->isThumb1Only())) {
    return fastEmitInst_rr(ARM::tSUBSrr, &ARM::tGPRRegClass, Op0, Op1);
  }
  if ((!Subtarget->isThumb())) {
    return fastEmitInst_rr(ARM::SUBSrr, &ARM::GPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_SUBS_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_SUBS_MVT_i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::UQADD16b.

unsigned fastEmit_ARMISD_UQADD16b_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasDSP()) && (Subtarget->isThumb2())) {
    return fastEmitInst_rr(ARM::t2UQADD16, &ARM::rGPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasV6Ops()) && (!Subtarget->isThumb())) {
    return fastEmitInst_rr(ARM::UQADD16, &ARM::GPRnopcRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_UQADD16b_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_UQADD16b_MVT_i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::UQADD8b.

unsigned fastEmit_ARMISD_UQADD8b_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasDSP()) && (Subtarget->isThumb2())) {
    return fastEmitInst_rr(ARM::t2UQADD8, &ARM::rGPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasV6Ops()) && (!Subtarget->isThumb())) {
    return fastEmitInst_rr(ARM::UQADD8, &ARM::GPRnopcRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_UQADD8b_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_UQADD8b_MVT_i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::UQSUB16b.

unsigned fastEmit_ARMISD_UQSUB16b_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasDSP()) && (Subtarget->isThumb2())) {
    return fastEmitInst_rr(ARM::t2UQSUB16, &ARM::rGPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasV6Ops()) && (!Subtarget->isThumb())) {
    return fastEmitInst_rr(ARM::UQSUB16, &ARM::GPRnopcRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_UQSUB16b_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_UQSUB16b_MVT_i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::UQSUB8b.

unsigned fastEmit_ARMISD_UQSUB8b_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasDSP()) && (Subtarget->isThumb2())) {
    return fastEmitInst_rr(ARM::t2UQSUB8, &ARM::rGPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasV6Ops()) && (!Subtarget->isThumb())) {
    return fastEmitInst_rr(ARM::UQSUB8, &ARM::GPRnopcRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_UQSUB8b_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_UQSUB8b_MVT_i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VMLAVs.

unsigned fastEmit_ARMISD_VMLAVs_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VMLADAVs8, &ARM::tGPREvenRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VMLAVs_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VMLADAVs16, &ARM::tGPREvenRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VMLAVs_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ARMISD_VMLAVs_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ARMISD_VMLAVs_MVT_v8i16_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VMLAVu.

unsigned fastEmit_ARMISD_VMLAVu_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VMLADAVu8, &ARM::tGPREvenRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VMLAVu_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VMLADAVu16, &ARM::tGPREvenRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VMLAVu_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ARMISD_VMLAVu_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ARMISD_VMLAVu_MVT_v8i16_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VMOVDRR.

unsigned fastEmit_ARMISD_VMOVDRR_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPRegs())) {
    return fastEmitInst_rr(ARM::VMOVDRR, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VMOVDRR_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_VMOVDRR_MVT_i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VMULLs.

unsigned fastEmit_ARMISD_VMULLs_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMULLsv8i16, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VMULLs_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMULLsv4i32, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VMULLs_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMULLsv2i64, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VMULLs_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VMULLBs32, &ARM::MQPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VMULLs_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ARMISD_VMULLs_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ARMISD_VMULLs_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ARMISD_VMULLs_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ARMISD_VMULLs_MVT_v4i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VMULLu.

unsigned fastEmit_ARMISD_VMULLu_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMULLuv8i16, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VMULLu_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMULLuv4i32, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VMULLu_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMULLuv2i64, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VMULLu_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VMULLBu32, &ARM::MQPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VMULLu_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ARMISD_VMULLu_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ARMISD_VMULLu_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ARMISD_VMULLu_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ARMISD_VMULLu_MVT_v4i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VQDMULH.

unsigned fastEmit_ARMISD_VQDMULH_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VQDMULHi8, &ARM::MQPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQDMULH_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VQDMULHi16, &ARM::MQPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQDMULH_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VQDMULHi32, &ARM::MQPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQDMULH_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ARMISD_VQDMULH_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ARMISD_VQDMULH_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ARMISD_VQDMULH_MVT_v4i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VSHLs.

unsigned fastEmit_ARMISD_VSHLs_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VSHLsv8i8, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHLs_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VSHL_by_vecs8, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VSHLsv16i8, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHLs_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VSHLsv4i16, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHLs_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VSHL_by_vecs16, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VSHLsv8i16, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHLs_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VSHLsv2i32, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHLs_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VSHL_by_vecs32, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VSHLsv4i32, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHLs_MVT_v1i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VSHLsv1i64, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHLs_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VSHLsv2i64, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHLs_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ARMISD_VSHLs_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_ARMISD_VSHLs_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ARMISD_VSHLs_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ARMISD_VSHLs_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ARMISD_VSHLs_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ARMISD_VSHLs_MVT_v4i32_rr(RetVT, Op0, Op1);
  case MVT::v1i64: return fastEmit_ARMISD_VSHLs_MVT_v1i64_rr(RetVT, Op0, Op1);
  case MVT::v2i64: return fastEmit_ARMISD_VSHLs_MVT_v2i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VSHLu.

unsigned fastEmit_ARMISD_VSHLu_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VSHLuv8i8, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHLu_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VSHL_by_vecu8, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VSHLuv16i8, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHLu_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VSHLuv4i16, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHLu_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VSHL_by_vecu16, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VSHLuv8i16, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHLu_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VSHLuv2i32, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHLu_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VSHL_by_vecu32, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VSHLuv4i32, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHLu_MVT_v1i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VSHLuv1i64, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHLu_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VSHLuv2i64, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHLu_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ARMISD_VSHLu_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_ARMISD_VSHLu_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ARMISD_VSHLu_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ARMISD_VSHLu_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ARMISD_VSHLu_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ARMISD_VSHLu_MVT_v4i32_rr(RetVT, Op0, Op1);
  case MVT::v1i64: return fastEmit_ARMISD_VSHLu_MVT_v1i64_rr(RetVT, Op0, Op1);
  case MVT::v2i64: return fastEmit_ARMISD_VSHLu_MVT_v2i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VTBL1.

unsigned fastEmit_ARMISD_VTBL1_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VTBL1, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VTBL1_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ARMISD_VTBL1_MVT_v8i8_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VTST.

unsigned fastEmit_ARMISD_VTST_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VTSTv8i8, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VTST_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VTSTv16i8, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VTST_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VTSTv4i16, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VTST_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VTSTv8i16, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VTST_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VTSTv2i32, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VTST_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VTSTv4i32, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VTST_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ARMISD_VTST_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_ARMISD_VTST_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ARMISD_VTST_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ARMISD_VTST_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ARMISD_VTST_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ARMISD_VTST_MVT_v4i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::ABDS.

unsigned fastEmit_ISD_ABDS_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VABDs8, &ARM::MQPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ABDS_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VABDs16, &ARM::MQPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ABDS_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VABDs32, &ARM::MQPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ABDS_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_ABDS_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_ABDS_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_ABDS_MVT_v4i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::ABDU.

unsigned fastEmit_ISD_ABDU_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VABDu8, &ARM::MQPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ABDU_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VABDu16, &ARM::MQPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ABDU_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VABDu32, &ARM::MQPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ABDU_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_ABDU_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_ABDU_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_ABDU_MVT_v4i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::ADD.

unsigned fastEmit_ISD_ADD_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb2())) {
    return fastEmitInst_rr(ARM::t2ADDrr, &ARM::GPRnopcRegClass, Op0, Op1);
  }
  if ((Subtarget->isThumb()) && (Subtarget->isThumb1Only())) {
    return fastEmitInst_rr(ARM::tADDrr, &ARM::tGPRRegClass, Op0, Op1);
  }
  if ((!Subtarget->isThumb())) {
    return fastEmitInst_rr(ARM::ADDrr, &ARM::GPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VADDv8i8, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VADDi8, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VADDv16i8, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VADDv4i16, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VADDi16, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VADDv8i16, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VADDv2i32, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VADDi32, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VADDv4i32, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v1i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VADDv1i64, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VADDv2i64, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_ADD_MVT_i32_rr(RetVT, Op0, Op1);
  case MVT::v8i8: return fastEmit_ISD_ADD_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_ISD_ADD_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ISD_ADD_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_ADD_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ISD_ADD_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_ADD_MVT_v4i32_rr(RetVT, Op0, Op1);
  case MVT::v1i64: return fastEmit_ISD_ADD_MVT_v1i64_rr(RetVT, Op0, Op1);
  case MVT::v2i64: return fastEmit_ISD_ADD_MVT_v2i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::AND.

unsigned fastEmit_ISD_AND_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb2())) {
    return fastEmitInst_rr(ARM::t2ANDrr, &ARM::rGPRRegClass, Op0, Op1);
  }
  if ((Subtarget->isThumb()) && (Subtarget->isThumb1Only())) {
    return fastEmitInst_rr(ARM::tAND, &ARM::tGPRRegClass, Op0, Op1);
  }
  if ((!Subtarget->isThumb())) {
    return fastEmitInst_rr(ARM::ANDrr, &ARM::GPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VANDd, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VAND, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VANDq, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VANDd, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VAND, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VANDq, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VANDd, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VAND, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VANDq, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v1i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VANDd, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VAND, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VANDq, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_AND_MVT_i32_rr(RetVT, Op0, Op1);
  case MVT::v8i8: return fastEmit_ISD_AND_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_ISD_AND_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ISD_AND_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_AND_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ISD_AND_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_AND_MVT_v4i32_rr(RetVT, Op0, Op1);
  case MVT::v1i64: return fastEmit_ISD_AND_MVT_v1i64_rr(RetVT, Op0, Op1);
  case MVT::v2i64: return fastEmit_ISD_AND_MVT_v2i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::AVGCEILS.

unsigned fastEmit_ISD_AVGCEILS_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  return fastEmitInst_rr(ARM::MVE_VRHADDs8, &ARM::MQPRRegClass, Op0, Op1);
}

unsigned fastEmit_ISD_AVGCEILS_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  return fastEmitInst_rr(ARM::MVE_VRHADDs16, &ARM::MQPRRegClass, Op0, Op1);
}

unsigned fastEmit_ISD_AVGCEILS_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  return fastEmitInst_rr(ARM::MVE_VRHADDs32, &ARM::MQPRRegClass, Op0, Op1);
}

unsigned fastEmit_ISD_AVGCEILS_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_AVGCEILS_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_AVGCEILS_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_AVGCEILS_MVT_v4i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::AVGCEILU.

unsigned fastEmit_ISD_AVGCEILU_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  return fastEmitInst_rr(ARM::MVE_VRHADDu8, &ARM::MQPRRegClass, Op0, Op1);
}

unsigned fastEmit_ISD_AVGCEILU_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  return fastEmitInst_rr(ARM::MVE_VRHADDu16, &ARM::MQPRRegClass, Op0, Op1);
}

unsigned fastEmit_ISD_AVGCEILU_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  return fastEmitInst_rr(ARM::MVE_VRHADDu32, &ARM::MQPRRegClass, Op0, Op1);
}

unsigned fastEmit_ISD_AVGCEILU_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_AVGCEILU_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_AVGCEILU_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_AVGCEILU_MVT_v4i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::AVGFLOORS.

unsigned fastEmit_ISD_AVGFLOORS_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  return fastEmitInst_rr(ARM::MVE_VHADDs8, &ARM::MQPRRegClass, Op0, Op1);
}

unsigned fastEmit_ISD_AVGFLOORS_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  return fastEmitInst_rr(ARM::MVE_VHADDs16, &ARM::MQPRRegClass, Op0, Op1);
}

unsigned fastEmit_ISD_AVGFLOORS_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  return fastEmitInst_rr(ARM::MVE_VHADDs32, &ARM::MQPRRegClass, Op0, Op1);
}

unsigned fastEmit_ISD_AVGFLOORS_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_AVGFLOORS_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_AVGFLOORS_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_AVGFLOORS_MVT_v4i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::AVGFLOORU.

unsigned fastEmit_ISD_AVGFLOORU_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  return fastEmitInst_rr(ARM::MVE_VHADDu8, &ARM::MQPRRegClass, Op0, Op1);
}

unsigned fastEmit_ISD_AVGFLOORU_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  return fastEmitInst_rr(ARM::MVE_VHADDu16, &ARM::MQPRRegClass, Op0, Op1);
}

unsigned fastEmit_ISD_AVGFLOORU_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  return fastEmitInst_rr(ARM::MVE_VHADDu32, &ARM::MQPRRegClass, Op0, Op1);
}

unsigned fastEmit_ISD_AVGFLOORU_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_AVGFLOORU_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_AVGFLOORU_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_AVGFLOORU_MVT_v4i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::FADD.

unsigned fastEmit_ISD_FADD_MVT_f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_rr(ARM::VADDH, &ARM::HPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((!Subtarget->useNEONForSinglePrecisionFP()) && (Subtarget->hasVFP2Base())) {
    return fastEmitInst_rr(ARM::VADDS, &ARM::SPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFP64()) && (Subtarget->hasVFP2Base())) {
    return fastEmitInst_rr(ARM::VADDD, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VADDhd, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasMVEFloatOps())) {
    return fastEmitInst_rr(ARM::MVE_VADDf16, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VADDhq, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VADDfd, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasMVEFloatOps())) {
    return fastEmitInst_rr(ARM::MVE_VADDf32, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VADDfq, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FADD_MVT_f16_rr(RetVT, Op0, Op1);
  case MVT::f32: return fastEmit_ISD_FADD_MVT_f32_rr(RetVT, Op0, Op1);
  case MVT::f64: return fastEmit_ISD_FADD_MVT_f64_rr(RetVT, Op0, Op1);
  case MVT::v4f16: return fastEmit_ISD_FADD_MVT_v4f16_rr(RetVT, Op0, Op1);
  case MVT::v8f16: return fastEmit_ISD_FADD_MVT_v8f16_rr(RetVT, Op0, Op1);
  case MVT::v2f32: return fastEmit_ISD_FADD_MVT_v2f32_rr(RetVT, Op0, Op1);
  case MVT::v4f32: return fastEmit_ISD_FADD_MVT_v4f32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::FDIV.

unsigned fastEmit_ISD_FDIV_MVT_f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_rr(ARM::VDIVH, &ARM::HPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FDIV_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasVFP2Base())) {
    return fastEmitInst_rr(ARM::VDIVS, &ARM::SPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FDIV_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFP64()) && (Subtarget->hasVFP2Base())) {
    return fastEmitInst_rr(ARM::VDIVD, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FDIV_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FDIV_MVT_f16_rr(RetVT, Op0, Op1);
  case MVT::f32: return fastEmit_ISD_FDIV_MVT_f32_rr(RetVT, Op0, Op1);
  case MVT::f64: return fastEmit_ISD_FDIV_MVT_f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::FMAXIMUM.

unsigned fastEmit_ISD_FMAXIMUM_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMAXhd, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMAXIMUM_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMAXhq, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMAXIMUM_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMAXfd, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMAXIMUM_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMAXfq, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMAXIMUM_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v4f16: return fastEmit_ISD_FMAXIMUM_MVT_v4f16_rr(RetVT, Op0, Op1);
  case MVT::v8f16: return fastEmit_ISD_FMAXIMUM_MVT_v8f16_rr(RetVT, Op0, Op1);
  case MVT::v2f32: return fastEmit_ISD_FMAXIMUM_MVT_v2f32_rr(RetVT, Op0, Op1);
  case MVT::v4f32: return fastEmit_ISD_FMAXIMUM_MVT_v4f32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::FMAXNUM.

unsigned fastEmit_ISD_FMAXNUM_MVT_f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_rr(ARM::VFP_VMAXNMH, &ARM::HPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMAXNUM_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8Base())) {
    return fastEmitInst_rr(ARM::VFP_VMAXNMS, &ARM::SPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMAXNUM_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFP64()) && (Subtarget->hasFPARMv8Base())) {
    return fastEmitInst_rr(ARM::VFP_VMAXNMD, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMAXNUM_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON()) && (Subtarget->hasV8Ops())) {
    return fastEmitInst_rr(ARM::NEON_VMAXNMNDh, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMAXNUM_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasMVEFloatOps())) {
    return fastEmitInst_rr(ARM::MVE_VMAXNMf16, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON()) && (Subtarget->hasV8Ops())) {
    return fastEmitInst_rr(ARM::NEON_VMAXNMNQh, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMAXNUM_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON()) && (Subtarget->hasV8Ops())) {
    return fastEmitInst_rr(ARM::NEON_VMAXNMNDf, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMAXNUM_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasMVEFloatOps())) {
    return fastEmitInst_rr(ARM::MVE_VMAXNMf32, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON()) && (Subtarget->hasV8Ops())) {
    return fastEmitInst_rr(ARM::NEON_VMAXNMNQf, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMAXNUM_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FMAXNUM_MVT_f16_rr(RetVT, Op0, Op1);
  case MVT::f32: return fastEmit_ISD_FMAXNUM_MVT_f32_rr(RetVT, Op0, Op1);
  case MVT::f64: return fastEmit_ISD_FMAXNUM_MVT_f64_rr(RetVT, Op0, Op1);
  case MVT::v4f16: return fastEmit_ISD_FMAXNUM_MVT_v4f16_rr(RetVT, Op0, Op1);
  case MVT::v8f16: return fastEmit_ISD_FMAXNUM_MVT_v8f16_rr(RetVT, Op0, Op1);
  case MVT::v2f32: return fastEmit_ISD_FMAXNUM_MVT_v2f32_rr(RetVT, Op0, Op1);
  case MVT::v4f32: return fastEmit_ISD_FMAXNUM_MVT_v4f32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::FMINIMUM.

unsigned fastEmit_ISD_FMINIMUM_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMINhd, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMINIMUM_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMINhq, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMINIMUM_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMINfd, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMINIMUM_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMINfq, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMINIMUM_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v4f16: return fastEmit_ISD_FMINIMUM_MVT_v4f16_rr(RetVT, Op0, Op1);
  case MVT::v8f16: return fastEmit_ISD_FMINIMUM_MVT_v8f16_rr(RetVT, Op0, Op1);
  case MVT::v2f32: return fastEmit_ISD_FMINIMUM_MVT_v2f32_rr(RetVT, Op0, Op1);
  case MVT::v4f32: return fastEmit_ISD_FMINIMUM_MVT_v4f32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::FMINNUM.

unsigned fastEmit_ISD_FMINNUM_MVT_f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_rr(ARM::VFP_VMINNMH, &ARM::HPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMINNUM_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8Base())) {
    return fastEmitInst_rr(ARM::VFP_VMINNMS, &ARM::SPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMINNUM_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFP64()) && (Subtarget->hasFPARMv8Base())) {
    return fastEmitInst_rr(ARM::VFP_VMINNMD, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMINNUM_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON()) && (Subtarget->hasV8Ops())) {
    return fastEmitInst_rr(ARM::NEON_VMINNMNDh, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMINNUM_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasMVEFloatOps())) {
    return fastEmitInst_rr(ARM::MVE_VMINNMf16, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON()) && (Subtarget->hasV8Ops())) {
    return fastEmitInst_rr(ARM::NEON_VMINNMNQh, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMINNUM_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON()) && (Subtarget->hasV8Ops())) {
    return fastEmitInst_rr(ARM::NEON_VMINNMNDf, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMINNUM_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasMVEFloatOps())) {
    return fastEmitInst_rr(ARM::MVE_VMINNMf32, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON()) && (Subtarget->hasV8Ops())) {
    return fastEmitInst_rr(ARM::NEON_VMINNMNQf, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMINNUM_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FMINNUM_MVT_f16_rr(RetVT, Op0, Op1);
  case MVT::f32: return fastEmit_ISD_FMINNUM_MVT_f32_rr(RetVT, Op0, Op1);
  case MVT::f64: return fastEmit_ISD_FMINNUM_MVT_f64_rr(RetVT, Op0, Op1);
  case MVT::v4f16: return fastEmit_ISD_FMINNUM_MVT_v4f16_rr(RetVT, Op0, Op1);
  case MVT::v8f16: return fastEmit_ISD_FMINNUM_MVT_v8f16_rr(RetVT, Op0, Op1);
  case MVT::v2f32: return fastEmit_ISD_FMINNUM_MVT_v2f32_rr(RetVT, Op0, Op1);
  case MVT::v4f32: return fastEmit_ISD_FMINNUM_MVT_v4f32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::FMUL.

unsigned fastEmit_ISD_FMUL_MVT_f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_rr(ARM::VMULH, &ARM::HPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((!Subtarget->useNEONForSinglePrecisionFP()) && (Subtarget->hasVFP2Base())) {
    return fastEmitInst_rr(ARM::VMULS, &ARM::SPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFP64()) && (Subtarget->hasVFP2Base())) {
    return fastEmitInst_rr(ARM::VMULD, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMULhd, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasMVEFloatOps())) {
    return fastEmitInst_rr(ARM::MVE_VMULf16, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMULhq, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMULfd, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasMVEFloatOps())) {
    return fastEmitInst_rr(ARM::MVE_VMULf32, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMULfq, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FMUL_MVT_f16_rr(RetVT, Op0, Op1);
  case MVT::f32: return fastEmit_ISD_FMUL_MVT_f32_rr(RetVT, Op0, Op1);
  case MVT::f64: return fastEmit_ISD_FMUL_MVT_f64_rr(RetVT, Op0, Op1);
  case MVT::v4f16: return fastEmit_ISD_FMUL_MVT_v4f16_rr(RetVT, Op0, Op1);
  case MVT::v8f16: return fastEmit_ISD_FMUL_MVT_v8f16_rr(RetVT, Op0, Op1);
  case MVT::v2f32: return fastEmit_ISD_FMUL_MVT_v2f32_rr(RetVT, Op0, Op1);
  case MVT::v4f32: return fastEmit_ISD_FMUL_MVT_v4f32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::FSUB.

unsigned fastEmit_ISD_FSUB_MVT_f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_rr(ARM::VSUBH, &ARM::HPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((!Subtarget->useNEONForSinglePrecisionFP()) && (Subtarget->hasVFP2Base())) {
    return fastEmitInst_rr(ARM::VSUBS, &ARM::SPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFP64()) && (Subtarget->hasVFP2Base())) {
    return fastEmitInst_rr(ARM::VSUBD, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VSUBhd, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasMVEFloatOps())) {
    return fastEmitInst_rr(ARM::MVE_VSUBf16, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VSUBhq, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VSUBfd, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasMVEFloatOps())) {
    return fastEmitInst_rr(ARM::MVE_VSUBf32, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VSUBfq, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FSUB_MVT_f16_rr(RetVT, Op0, Op1);
  case MVT::f32: return fastEmit_ISD_FSUB_MVT_f32_rr(RetVT, Op0, Op1);
  case MVT::f64: return fastEmit_ISD_FSUB_MVT_f64_rr(RetVT, Op0, Op1);
  case MVT::v4f16: return fastEmit_ISD_FSUB_MVT_v4f16_rr(RetVT, Op0, Op1);
  case MVT::v8f16: return fastEmit_ISD_FSUB_MVT_v8f16_rr(RetVT, Op0, Op1);
  case MVT::v2f32: return fastEmit_ISD_FSUB_MVT_v2f32_rr(RetVT, Op0, Op1);
  case MVT::v4f32: return fastEmit_ISD_FSUB_MVT_v4f32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::MUL.

unsigned fastEmit_ISD_MUL_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb2())) {
    return fastEmitInst_rr(ARM::t2MUL, &ARM::rGPRRegClass, Op0, Op1);
  }
  if ((Subtarget->isThumb()) && (Subtarget->isThumb1Only())) {
    return fastEmitInst_rr(ARM::tMUL, &ARM::tGPRRegClass, Op0, Op1);
  }
  if ((!Subtarget->isThumb()) && (!Subtarget->hasV6Ops()) && (Subtarget->useMulOps())) {
    return fastEmitInst_rr(ARM::MULv5, &ARM::GPRnopcRegClass, Op0, Op1);
  }
  if ((Subtarget->hasV6Ops()) && (!Subtarget->isThumb())) {
    return fastEmitInst_rr(ARM::MUL, &ARM::GPRnopcRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_MUL_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMULv8i8, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_MUL_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VMULi8, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMULv16i8, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_MUL_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMULv4i16, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_MUL_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VMULi16, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMULv8i16, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_MUL_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMULv2i32, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_MUL_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VMULi32, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMULv4i32, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_MUL_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_MUL_MVT_i32_rr(RetVT, Op0, Op1);
  case MVT::v8i8: return fastEmit_ISD_MUL_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_ISD_MUL_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ISD_MUL_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_MUL_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ISD_MUL_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_MUL_MVT_v4i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::MULHS.

unsigned fastEmit_ISD_MULHS_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasDSP()) && (Subtarget->isThumb2())) {
    return fastEmitInst_rr(ARM::t2SMMUL, &ARM::rGPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasV6Ops()) && (!Subtarget->isThumb())) {
    return fastEmitInst_rr(ARM::SMMUL, &ARM::GPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_MULHS_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VMULHs8, &ARM::MQPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_MULHS_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VMULHs16, &ARM::MQPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_MULHS_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VMULHs32, &ARM::MQPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_MULHS_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_MULHS_MVT_i32_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_ISD_MULHS_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_MULHS_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_MULHS_MVT_v4i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::MULHU.

unsigned fastEmit_ISD_MULHU_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VMULHu8, &ARM::MQPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_MULHU_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VMULHu16, &ARM::MQPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_MULHU_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VMULHu32, &ARM::MQPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_MULHU_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_MULHU_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_MULHU_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_MULHU_MVT_v4i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::OR.

unsigned fastEmit_ISD_OR_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb2())) {
    return fastEmitInst_rr(ARM::t2ORRrr, &ARM::rGPRRegClass, Op0, Op1);
  }
  if ((Subtarget->isThumb()) && (Subtarget->isThumb1Only())) {
    return fastEmitInst_rr(ARM::tORR, &ARM::tGPRRegClass, Op0, Op1);
  }
  if ((!Subtarget->isThumb())) {
    return fastEmitInst_rr(ARM::ORRrr, &ARM::GPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VORRd, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VORR, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VORRq, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VORRd, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VORR, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VORRq, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VORRd, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VORR, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VORRq, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v1i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VORRd, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VORR, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VORRq, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_OR_MVT_i32_rr(RetVT, Op0, Op1);
  case MVT::v8i8: return fastEmit_ISD_OR_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_ISD_OR_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ISD_OR_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_OR_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ISD_OR_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_OR_MVT_v4i32_rr(RetVT, Op0, Op1);
  case MVT::v1i64: return fastEmit_ISD_OR_MVT_v1i64_rr(RetVT, Op0, Op1);
  case MVT::v2i64: return fastEmit_ISD_OR_MVT_v2i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::ROTR.

unsigned fastEmit_ISD_ROTR_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb2())) {
    return fastEmitInst_rr(ARM::t2RORrr, &ARM::rGPRRegClass, Op0, Op1);
  }
  if ((Subtarget->isThumb()) && (Subtarget->isThumb1Only())) {
    return fastEmitInst_rr(ARM::tROR, &ARM::tGPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ROTR_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_ROTR_MVT_i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SADDSAT.

unsigned fastEmit_ISD_SADDSAT_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasDSP()) && (Subtarget->isThumb2())) {
    return fastEmitInst_rr(ARM::t2QADD, &ARM::rGPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasV5TEOps()) && (!Subtarget->isThumb())) {
    return fastEmitInst_rr(ARM::QADD, &ARM::GPRnopcRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SADDSAT_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQADDsv8i8, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SADDSAT_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VQADDs8, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQADDsv16i8, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SADDSAT_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQADDsv4i16, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SADDSAT_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VQADDs16, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQADDsv8i16, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SADDSAT_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQADDsv2i32, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SADDSAT_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VQADDs32, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQADDsv4i32, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SADDSAT_MVT_v1i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQADDsv1i64, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SADDSAT_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQADDsv2i64, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SADDSAT_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SADDSAT_MVT_i32_rr(RetVT, Op0, Op1);
  case MVT::v8i8: return fastEmit_ISD_SADDSAT_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_ISD_SADDSAT_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ISD_SADDSAT_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_SADDSAT_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ISD_SADDSAT_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_SADDSAT_MVT_v4i32_rr(RetVT, Op0, Op1);
  case MVT::v1i64: return fastEmit_ISD_SADDSAT_MVT_v1i64_rr(RetVT, Op0, Op1);
  case MVT::v2i64: return fastEmit_ISD_SADDSAT_MVT_v2i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SDIV.

unsigned fastEmit_ISD_SDIV_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasDivideInThumbMode()) && (Subtarget->hasV8MBaselineOps()) && (Subtarget->isThumb())) {
    return fastEmitInst_rr(ARM::t2SDIV, &ARM::rGPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasDivideInARMMode()) && (!Subtarget->isThumb())) {
    return fastEmitInst_rr(ARM::SDIV, &ARM::GPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SDIV_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SDIV_MVT_i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SHL.

unsigned fastEmit_ISD_SHL_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb2())) {
    return fastEmitInst_rr(ARM::t2LSLrr, &ARM::rGPRRegClass, Op0, Op1);
  }
  if ((Subtarget->isThumb()) && (Subtarget->isThumb1Only())) {
    return fastEmitInst_rr(ARM::tLSLrr, &ARM::tGPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SHL_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SHL_MVT_i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SMAX.

unsigned fastEmit_ISD_SMAX_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMAXsv8i8, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VMAXs8, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMAXsv16i8, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMAXsv4i16, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VMAXs16, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMAXsv8i16, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMAXsv2i32, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VMAXs32, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMAXsv4i32, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ISD_SMAX_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_ISD_SMAX_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ISD_SMAX_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_SMAX_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ISD_SMAX_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_SMAX_MVT_v4i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SMIN.

unsigned fastEmit_ISD_SMIN_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMINsv8i8, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VMINs8, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMINsv16i8, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMINsv4i16, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VMINs16, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMINsv8i16, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMINsv2i32, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VMINs32, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMINsv4i32, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ISD_SMIN_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_ISD_SMIN_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ISD_SMIN_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_SMIN_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ISD_SMIN_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_SMIN_MVT_v4i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SRA.

unsigned fastEmit_ISD_SRA_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb2())) {
    return fastEmitInst_rr(ARM::t2ASRrr, &ARM::rGPRRegClass, Op0, Op1);
  }
  if ((Subtarget->isThumb()) && (Subtarget->isThumb1Only())) {
    return fastEmitInst_rr(ARM::tASRrr, &ARM::tGPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SRA_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SRA_MVT_i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SRL.

unsigned fastEmit_ISD_SRL_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb2())) {
    return fastEmitInst_rr(ARM::t2LSRrr, &ARM::rGPRRegClass, Op0, Op1);
  }
  if ((Subtarget->isThumb()) && (Subtarget->isThumb1Only())) {
    return fastEmitInst_rr(ARM::tLSRrr, &ARM::tGPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SRL_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SRL_MVT_i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SSUBSAT.

unsigned fastEmit_ISD_SSUBSAT_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasDSP()) && (Subtarget->isThumb2())) {
    return fastEmitInst_rr(ARM::t2QSUB, &ARM::rGPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasV5TEOps()) && (!Subtarget->isThumb())) {
    return fastEmitInst_rr(ARM::QSUB, &ARM::GPRnopcRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SSUBSAT_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQSUBsv8i8, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SSUBSAT_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VQSUBs8, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQSUBsv16i8, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SSUBSAT_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQSUBsv4i16, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SSUBSAT_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VQSUBs16, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQSUBsv8i16, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SSUBSAT_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQSUBsv2i32, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SSUBSAT_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VQSUBs32, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQSUBsv4i32, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SSUBSAT_MVT_v1i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQSUBsv1i64, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SSUBSAT_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQSUBsv2i64, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SSUBSAT_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SSUBSAT_MVT_i32_rr(RetVT, Op0, Op1);
  case MVT::v8i8: return fastEmit_ISD_SSUBSAT_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_ISD_SSUBSAT_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ISD_SSUBSAT_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_SSUBSAT_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ISD_SSUBSAT_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_SSUBSAT_MVT_v4i32_rr(RetVT, Op0, Op1);
  case MVT::v1i64: return fastEmit_ISD_SSUBSAT_MVT_v1i64_rr(RetVT, Op0, Op1);
  case MVT::v2i64: return fastEmit_ISD_SSUBSAT_MVT_v2i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SUB.

unsigned fastEmit_ISD_SUB_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb2())) {
    return fastEmitInst_rr(ARM::t2SUBrr, &ARM::GPRnopcRegClass, Op0, Op1);
  }
  if ((Subtarget->isThumb()) && (Subtarget->isThumb1Only())) {
    return fastEmitInst_rr(ARM::tSUBrr, &ARM::tGPRRegClass, Op0, Op1);
  }
  if ((!Subtarget->isThumb())) {
    return fastEmitInst_rr(ARM::SUBrr, &ARM::GPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VSUBv8i8, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VSUBi8, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VSUBv16i8, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VSUBv4i16, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VSUBi16, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VSUBv8i16, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VSUBv2i32, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VSUBi32, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VSUBv4i32, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v1i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VSUBv1i64, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VSUBv2i64, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SUB_MVT_i32_rr(RetVT, Op0, Op1);
  case MVT::v8i8: return fastEmit_ISD_SUB_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_ISD_SUB_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ISD_SUB_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_SUB_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ISD_SUB_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_SUB_MVT_v4i32_rr(RetVT, Op0, Op1);
  case MVT::v1i64: return fastEmit_ISD_SUB_MVT_v1i64_rr(RetVT, Op0, Op1);
  case MVT::v2i64: return fastEmit_ISD_SUB_MVT_v2i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::UADDSAT.

unsigned fastEmit_ISD_UADDSAT_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQADDuv8i8, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UADDSAT_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VQADDu8, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQADDuv16i8, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UADDSAT_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQADDuv4i16, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UADDSAT_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VQADDu16, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQADDuv8i16, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UADDSAT_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQADDuv2i32, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UADDSAT_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VQADDu32, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQADDuv4i32, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UADDSAT_MVT_v1i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQADDuv1i64, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UADDSAT_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQADDuv2i64, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UADDSAT_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ISD_UADDSAT_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_ISD_UADDSAT_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ISD_UADDSAT_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_UADDSAT_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ISD_UADDSAT_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_UADDSAT_MVT_v4i32_rr(RetVT, Op0, Op1);
  case MVT::v1i64: return fastEmit_ISD_UADDSAT_MVT_v1i64_rr(RetVT, Op0, Op1);
  case MVT::v2i64: return fastEmit_ISD_UADDSAT_MVT_v2i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::UDIV.

unsigned fastEmit_ISD_UDIV_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasDivideInThumbMode()) && (Subtarget->hasV8MBaselineOps()) && (Subtarget->isThumb())) {
    return fastEmitInst_rr(ARM::t2UDIV, &ARM::rGPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasDivideInARMMode()) && (!Subtarget->isThumb())) {
    return fastEmitInst_rr(ARM::UDIV, &ARM::GPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UDIV_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_UDIV_MVT_i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::UMAX.

unsigned fastEmit_ISD_UMAX_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMAXuv8i8, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VMAXu8, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMAXuv16i8, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMAXuv4i16, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VMAXu16, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMAXuv8i16, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMAXuv2i32, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VMAXu32, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMAXuv4i32, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ISD_UMAX_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_ISD_UMAX_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ISD_UMAX_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_UMAX_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ISD_UMAX_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_UMAX_MVT_v4i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::UMIN.

unsigned fastEmit_ISD_UMIN_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMINuv8i8, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VMINu8, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMINuv16i8, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMINuv4i16, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VMINu16, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMINuv8i16, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMINuv2i32, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VMINu32, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VMINuv4i32, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ISD_UMIN_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_ISD_UMIN_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ISD_UMIN_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_UMIN_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ISD_UMIN_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_UMIN_MVT_v4i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::USUBSAT.

unsigned fastEmit_ISD_USUBSAT_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQSUBuv8i8, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_USUBSAT_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VQSUBu8, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQSUBuv16i8, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_USUBSAT_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQSUBuv4i16, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_USUBSAT_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VQSUBu16, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQSUBuv8i16, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_USUBSAT_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQSUBuv2i32, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_USUBSAT_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VQSUBu32, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQSUBuv4i32, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_USUBSAT_MVT_v1i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQSUBuv1i64, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_USUBSAT_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VQSUBuv2i64, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_USUBSAT_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ISD_USUBSAT_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_ISD_USUBSAT_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ISD_USUBSAT_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_USUBSAT_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ISD_USUBSAT_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_USUBSAT_MVT_v4i32_rr(RetVT, Op0, Op1);
  case MVT::v1i64: return fastEmit_ISD_USUBSAT_MVT_v1i64_rr(RetVT, Op0, Op1);
  case MVT::v2i64: return fastEmit_ISD_USUBSAT_MVT_v2i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::XOR.

unsigned fastEmit_ISD_XOR_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb2())) {
    return fastEmitInst_rr(ARM::t2EORrr, &ARM::rGPRRegClass, Op0, Op1);
  }
  if ((Subtarget->isThumb()) && (Subtarget->isThumb1Only())) {
    return fastEmitInst_rr(ARM::tEOR, &ARM::tGPRRegClass, Op0, Op1);
  }
  if ((!Subtarget->isThumb())) {
    return fastEmitInst_rr(ARM::EORrr, &ARM::GPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VEORd, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VEOR, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VEORq, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VEORd, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VEOR, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VEORq, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VEORd, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VEOR, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VEORq, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v1i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VEORd, &ARM::DPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_rr(ARM::MVE_VEOR, &ARM::MQPRRegClass, Op0, Op1);
  }
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(ARM::VEORq, &ARM::QPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_XOR_MVT_i32_rr(RetVT, Op0, Op1);
  case MVT::v8i8: return fastEmit_ISD_XOR_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_ISD_XOR_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ISD_XOR_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_XOR_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ISD_XOR_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_XOR_MVT_v4i32_rr(RetVT, Op0, Op1);
  case MVT::v1i64: return fastEmit_ISD_XOR_MVT_v1i64_rr(RetVT, Op0, Op1);
  case MVT::v2i64: return fastEmit_ISD_XOR_MVT_v2i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_rr(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, unsigned Op1) override {
  switch (Opcode) {
  case ARMISD::CMP: return fastEmit_ARMISD_CMP_rr(VT, RetVT, Op0, Op1);
  case ARMISD::CMPFP: return fastEmit_ARMISD_CMPFP_rr(VT, RetVT, Op0, Op1);
  case ARMISD::CMPFPE: return fastEmit_ARMISD_CMPFPE_rr(VT, RetVT, Op0, Op1);
  case ARMISD::CMPZ: return fastEmit_ARMISD_CMPZ_rr(VT, RetVT, Op0, Op1);
  case ARMISD::EH_SJLJ_LONGJMP: return fastEmit_ARMISD_EH_SJLJ_LONGJMP_rr(VT, RetVT, Op0, Op1);
  case ARMISD::EH_SJLJ_SETJMP: return fastEmit_ARMISD_EH_SJLJ_SETJMP_rr(VT, RetVT, Op0, Op1);
  case ARMISD::QADD16b: return fastEmit_ARMISD_QADD16b_rr(VT, RetVT, Op0, Op1);
  case ARMISD::QADD8b: return fastEmit_ARMISD_QADD8b_rr(VT, RetVT, Op0, Op1);
  case ARMISD::QSUB16b: return fastEmit_ARMISD_QSUB16b_rr(VT, RetVT, Op0, Op1);
  case ARMISD::QSUB8b: return fastEmit_ARMISD_QSUB8b_rr(VT, RetVT, Op0, Op1);
  case ARMISD::SMULWB: return fastEmit_ARMISD_SMULWB_rr(VT, RetVT, Op0, Op1);
  case ARMISD::SMULWT: return fastEmit_ARMISD_SMULWT_rr(VT, RetVT, Op0, Op1);
  case ARMISD::SUBS: return fastEmit_ARMISD_SUBS_rr(VT, RetVT, Op0, Op1);
  case ARMISD::UQADD16b: return fastEmit_ARMISD_UQADD16b_rr(VT, RetVT, Op0, Op1);
  case ARMISD::UQADD8b: return fastEmit_ARMISD_UQADD8b_rr(VT, RetVT, Op0, Op1);
  case ARMISD::UQSUB16b: return fastEmit_ARMISD_UQSUB16b_rr(VT, RetVT, Op0, Op1);
  case ARMISD::UQSUB8b: return fastEmit_ARMISD_UQSUB8b_rr(VT, RetVT, Op0, Op1);
  case ARMISD::VMLAVs: return fastEmit_ARMISD_VMLAVs_rr(VT, RetVT, Op0, Op1);
  case ARMISD::VMLAVu: return fastEmit_ARMISD_VMLAVu_rr(VT, RetVT, Op0, Op1);
  case ARMISD::VMOVDRR: return fastEmit_ARMISD_VMOVDRR_rr(VT, RetVT, Op0, Op1);
  case ARMISD::VMULLs: return fastEmit_ARMISD_VMULLs_rr(VT, RetVT, Op0, Op1);
  case ARMISD::VMULLu: return fastEmit_ARMISD_VMULLu_rr(VT, RetVT, Op0, Op1);
  case ARMISD::VQDMULH: return fastEmit_ARMISD_VQDMULH_rr(VT, RetVT, Op0, Op1);
  case ARMISD::VSHLs: return fastEmit_ARMISD_VSHLs_rr(VT, RetVT, Op0, Op1);
  case ARMISD::VSHLu: return fastEmit_ARMISD_VSHLu_rr(VT, RetVT, Op0, Op1);
  case ARMISD::VTBL1: return fastEmit_ARMISD_VTBL1_rr(VT, RetVT, Op0, Op1);
  case ARMISD::VTST: return fastEmit_ARMISD_VTST_rr(VT, RetVT, Op0, Op1);
  case ISD::ABDS: return fastEmit_ISD_ABDS_rr(VT, RetVT, Op0, Op1);
  case ISD::ABDU: return fastEmit_ISD_ABDU_rr(VT, RetVT, Op0, Op1);
  case ISD::ADD: return fastEmit_ISD_ADD_rr(VT, RetVT, Op0, Op1);
  case ISD::AND: return fastEmit_ISD_AND_rr(VT, RetVT, Op0, Op1);
  case ISD::AVGCEILS: return fastEmit_ISD_AVGCEILS_rr(VT, RetVT, Op0, Op1);
  case ISD::AVGCEILU: return fastEmit_ISD_AVGCEILU_rr(VT, RetVT, Op0, Op1);
  case ISD::AVGFLOORS: return fastEmit_ISD_AVGFLOORS_rr(VT, RetVT, Op0, Op1);
  case ISD::AVGFLOORU: return fastEmit_ISD_AVGFLOORU_rr(VT, RetVT, Op0, Op1);
  case ISD::FADD: return fastEmit_ISD_FADD_rr(VT, RetVT, Op0, Op1);
  case ISD::FDIV: return fastEmit_ISD_FDIV_rr(VT, RetVT, Op0, Op1);
  case ISD::FMAXIMUM: return fastEmit_ISD_FMAXIMUM_rr(VT, RetVT, Op0, Op1);
  case ISD::FMAXNUM: return fastEmit_ISD_FMAXNUM_rr(VT, RetVT, Op0, Op1);
  case ISD::FMINIMUM: return fastEmit_ISD_FMINIMUM_rr(VT, RetVT, Op0, Op1);
  case ISD::FMINNUM: return fastEmit_ISD_FMINNUM_rr(VT, RetVT, Op0, Op1);
  case ISD::FMUL: return fastEmit_ISD_FMUL_rr(VT, RetVT, Op0, Op1);
  case ISD::FSUB: return fastEmit_ISD_FSUB_rr(VT, RetVT, Op0, Op1);
  case ISD::MUL: return fastEmit_ISD_MUL_rr(VT, RetVT, Op0, Op1);
  case ISD::MULHS: return fastEmit_ISD_MULHS_rr(VT, RetVT, Op0, Op1);
  case ISD::MULHU: return fastEmit_ISD_MULHU_rr(VT, RetVT, Op0, Op1);
  case ISD::OR: return fastEmit_ISD_OR_rr(VT, RetVT, Op0, Op1);
  case ISD::ROTR: return fastEmit_ISD_ROTR_rr(VT, RetVT, Op0, Op1);
  case ISD::SADDSAT: return fastEmit_ISD_SADDSAT_rr(VT, RetVT, Op0, Op1);
  case ISD::SDIV: return fastEmit_ISD_SDIV_rr(VT, RetVT, Op0, Op1);
  case ISD::SHL: return fastEmit_ISD_SHL_rr(VT, RetVT, Op0, Op1);
  case ISD::SMAX: return fastEmit_ISD_SMAX_rr(VT, RetVT, Op0, Op1);
  case ISD::SMIN: return fastEmit_ISD_SMIN_rr(VT, RetVT, Op0, Op1);
  case ISD::SRA: return fastEmit_ISD_SRA_rr(VT, RetVT, Op0, Op1);
  case ISD::SRL: return fastEmit_ISD_SRL_rr(VT, RetVT, Op0, Op1);
  case ISD::SSUBSAT: return fastEmit_ISD_SSUBSAT_rr(VT, RetVT, Op0, Op1);
  case ISD::SUB: return fastEmit_ISD_SUB_rr(VT, RetVT, Op0, Op1);
  case ISD::UADDSAT: return fastEmit_ISD_UADDSAT_rr(VT, RetVT, Op0, Op1);
  case ISD::UDIV: return fastEmit_ISD_UDIV_rr(VT, RetVT, Op0, Op1);
  case ISD::UMAX: return fastEmit_ISD_UMAX_rr(VT, RetVT, Op0, Op1);
  case ISD::UMIN: return fastEmit_ISD_UMIN_rr(VT, RetVT, Op0, Op1);
  case ISD::USUBSAT: return fastEmit_ISD_USUBSAT_rr(VT, RetVT, Op0, Op1);
  case ISD::XOR: return fastEmit_ISD_XOR_rr(VT, RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::PIC_ADD.

unsigned fastEmit_ARMISD_PIC_ADD_MVT_i32_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb())) {
    return fastEmitInst_ri(ARM::tPICADD, &ARM::GPRRegClass, Op0, imm1);
  }
  if ((!Subtarget->isThumb())) {
    return fastEmitInst_ri(ARM::PICADD, &ARM::GPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_PIC_ADD_ri(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_PIC_ADD_MVT_i32_ri(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VDUPLANE.

unsigned fastEmit_ARMISD_VDUPLANE_MVT_v8i8_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VDUPLN8d, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VDUPLANE_MVT_v4i16_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VDUPLN16d, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VDUPLANE_MVT_v2i32_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VDUPLN32d, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VDUPLANE_MVT_v4f16_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VDUPLN16d, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VDUPLANE_MVT_v4bf16_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4bf16)
    return 0;
  if ((Subtarget->hasBF16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VDUPLN16d, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VDUPLANE_MVT_v2f32_MVT_v2f32_ri(unsigned Op0, uint64_t imm1) {
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VDUPLN32d, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VDUPLANE_MVT_v2f32_MVT_v4f32_ri(unsigned Op0, uint64_t imm1) {
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VDUPLN32q, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VDUPLANE_MVT_v2f32_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
switch (RetVT.SimpleTy) {
  case MVT::v2f32: return fastEmit_ARMISD_VDUPLANE_MVT_v2f32_MVT_v2f32_ri(Op0, imm1);
  case MVT::v4f32: return fastEmit_ARMISD_VDUPLANE_MVT_v2f32_MVT_v4f32_ri(Op0, imm1);
  default: return 0;
}
}

unsigned fastEmit_ARMISD_VDUPLANE_ri(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ARMISD_VDUPLANE_MVT_v8i8_ri(RetVT, Op0, imm1);
  case MVT::v4i16: return fastEmit_ARMISD_VDUPLANE_MVT_v4i16_ri(RetVT, Op0, imm1);
  case MVT::v2i32: return fastEmit_ARMISD_VDUPLANE_MVT_v2i32_ri(RetVT, Op0, imm1);
  case MVT::v4f16: return fastEmit_ARMISD_VDUPLANE_MVT_v4f16_ri(RetVT, Op0, imm1);
  case MVT::v4bf16: return fastEmit_ARMISD_VDUPLANE_MVT_v4bf16_ri(RetVT, Op0, imm1);
  case MVT::v2f32: return fastEmit_ARMISD_VDUPLANE_MVT_v2f32_ri(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VGETLANEs.

unsigned fastEmit_ARMISD_VGETLANEs_MVT_v8i8_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VGETLNs8, &ARM::GPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VGETLANEs_MVT_v16i8_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_ri(ARM::MVE_VMOV_from_lane_s8, &ARM::rGPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VGETLANEs_MVT_v4i16_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VGETLNs16, &ARM::GPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VGETLANEs_MVT_v8i16_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_ri(ARM::MVE_VMOV_from_lane_s16, &ARM::rGPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VGETLANEs_MVT_v8f16_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_ri(ARM::MVE_VMOV_from_lane_s16, &ARM::rGPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VGETLANEs_ri(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ARMISD_VGETLANEs_MVT_v8i8_ri(RetVT, Op0, imm1);
  case MVT::v16i8: return fastEmit_ARMISD_VGETLANEs_MVT_v16i8_ri(RetVT, Op0, imm1);
  case MVT::v4i16: return fastEmit_ARMISD_VGETLANEs_MVT_v4i16_ri(RetVT, Op0, imm1);
  case MVT::v8i16: return fastEmit_ARMISD_VGETLANEs_MVT_v8i16_ri(RetVT, Op0, imm1);
  case MVT::v8f16: return fastEmit_ARMISD_VGETLANEs_MVT_v8f16_ri(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VGETLANEu.

unsigned fastEmit_ARMISD_VGETLANEu_MVT_v8i8_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VGETLNu8, &ARM::GPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VGETLANEu_MVT_v16i8_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_ri(ARM::MVE_VMOV_from_lane_u8, &ARM::rGPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VGETLANEu_MVT_v4i16_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VGETLNu16, &ARM::GPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VGETLANEu_MVT_v8i16_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_ri(ARM::MVE_VMOV_from_lane_u16, &ARM::rGPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VGETLANEu_MVT_v4f16_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VGETLNu16, &ARM::GPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VGETLANEu_MVT_v8f16_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_ri(ARM::MVE_VMOV_from_lane_u16, &ARM::rGPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VGETLANEu_MVT_v4bf16_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VGETLNu16, &ARM::GPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VGETLANEu_ri(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ARMISD_VGETLANEu_MVT_v8i8_ri(RetVT, Op0, imm1);
  case MVT::v16i8: return fastEmit_ARMISD_VGETLANEu_MVT_v16i8_ri(RetVT, Op0, imm1);
  case MVT::v4i16: return fastEmit_ARMISD_VGETLANEu_MVT_v4i16_ri(RetVT, Op0, imm1);
  case MVT::v8i16: return fastEmit_ARMISD_VGETLANEu_MVT_v8i16_ri(RetVT, Op0, imm1);
  case MVT::v4f16: return fastEmit_ARMISD_VGETLANEu_MVT_v4f16_ri(RetVT, Op0, imm1);
  case MVT::v8f16: return fastEmit_ARMISD_VGETLANEu_MVT_v8f16_ri(RetVT, Op0, imm1);
  case MVT::v4bf16: return fastEmit_ARMISD_VGETLANEu_MVT_v4bf16_ri(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VQSHLsIMM.

unsigned fastEmit_ARMISD_VQSHLsIMM_MVT_v8i8_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHLsiv8i8, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHLsIMM_MVT_v16i8_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHLsiv16i8, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHLsIMM_MVT_v4i16_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHLsiv4i16, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHLsIMM_MVT_v8i16_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHLsiv8i16, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHLsIMM_MVT_v2i32_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHLsiv2i32, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHLsIMM_MVT_v4i32_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHLsiv4i32, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHLsIMM_MVT_v1i64_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHLsiv1i64, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHLsIMM_MVT_v2i64_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHLsiv2i64, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHLsIMM_ri(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ARMISD_VQSHLsIMM_MVT_v8i8_ri(RetVT, Op0, imm1);
  case MVT::v16i8: return fastEmit_ARMISD_VQSHLsIMM_MVT_v16i8_ri(RetVT, Op0, imm1);
  case MVT::v4i16: return fastEmit_ARMISD_VQSHLsIMM_MVT_v4i16_ri(RetVT, Op0, imm1);
  case MVT::v8i16: return fastEmit_ARMISD_VQSHLsIMM_MVT_v8i16_ri(RetVT, Op0, imm1);
  case MVT::v2i32: return fastEmit_ARMISD_VQSHLsIMM_MVT_v2i32_ri(RetVT, Op0, imm1);
  case MVT::v4i32: return fastEmit_ARMISD_VQSHLsIMM_MVT_v4i32_ri(RetVT, Op0, imm1);
  case MVT::v1i64: return fastEmit_ARMISD_VQSHLsIMM_MVT_v1i64_ri(RetVT, Op0, imm1);
  case MVT::v2i64: return fastEmit_ARMISD_VQSHLsIMM_MVT_v2i64_ri(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VQSHLsuIMM.

unsigned fastEmit_ARMISD_VQSHLsuIMM_MVT_v8i8_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHLsuv8i8, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHLsuIMM_MVT_v16i8_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHLsuv16i8, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHLsuIMM_MVT_v4i16_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHLsuv4i16, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHLsuIMM_MVT_v8i16_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHLsuv8i16, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHLsuIMM_MVT_v2i32_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHLsuv2i32, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHLsuIMM_MVT_v4i32_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHLsuv4i32, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHLsuIMM_MVT_v1i64_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHLsuv1i64, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHLsuIMM_MVT_v2i64_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHLsuv2i64, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHLsuIMM_ri(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ARMISD_VQSHLsuIMM_MVT_v8i8_ri(RetVT, Op0, imm1);
  case MVT::v16i8: return fastEmit_ARMISD_VQSHLsuIMM_MVT_v16i8_ri(RetVT, Op0, imm1);
  case MVT::v4i16: return fastEmit_ARMISD_VQSHLsuIMM_MVT_v4i16_ri(RetVT, Op0, imm1);
  case MVT::v8i16: return fastEmit_ARMISD_VQSHLsuIMM_MVT_v8i16_ri(RetVT, Op0, imm1);
  case MVT::v2i32: return fastEmit_ARMISD_VQSHLsuIMM_MVT_v2i32_ri(RetVT, Op0, imm1);
  case MVT::v4i32: return fastEmit_ARMISD_VQSHLsuIMM_MVT_v4i32_ri(RetVT, Op0, imm1);
  case MVT::v1i64: return fastEmit_ARMISD_VQSHLsuIMM_MVT_v1i64_ri(RetVT, Op0, imm1);
  case MVT::v2i64: return fastEmit_ARMISD_VQSHLsuIMM_MVT_v2i64_ri(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VQSHLuIMM.

unsigned fastEmit_ARMISD_VQSHLuIMM_MVT_v8i8_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHLuiv8i8, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHLuIMM_MVT_v16i8_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHLuiv16i8, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHLuIMM_MVT_v4i16_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHLuiv4i16, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHLuIMM_MVT_v8i16_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHLuiv8i16, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHLuIMM_MVT_v2i32_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHLuiv2i32, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHLuIMM_MVT_v4i32_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHLuiv4i32, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHLuIMM_MVT_v1i64_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHLuiv1i64, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHLuIMM_MVT_v2i64_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHLuiv2i64, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHLuIMM_ri(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ARMISD_VQSHLuIMM_MVT_v8i8_ri(RetVT, Op0, imm1);
  case MVT::v16i8: return fastEmit_ARMISD_VQSHLuIMM_MVT_v16i8_ri(RetVT, Op0, imm1);
  case MVT::v4i16: return fastEmit_ARMISD_VQSHLuIMM_MVT_v4i16_ri(RetVT, Op0, imm1);
  case MVT::v8i16: return fastEmit_ARMISD_VQSHLuIMM_MVT_v8i16_ri(RetVT, Op0, imm1);
  case MVT::v2i32: return fastEmit_ARMISD_VQSHLuIMM_MVT_v2i32_ri(RetVT, Op0, imm1);
  case MVT::v4i32: return fastEmit_ARMISD_VQSHLuIMM_MVT_v4i32_ri(RetVT, Op0, imm1);
  case MVT::v1i64: return fastEmit_ARMISD_VQSHLuIMM_MVT_v1i64_ri(RetVT, Op0, imm1);
  case MVT::v2i64: return fastEmit_ARMISD_VQSHLuIMM_MVT_v2i64_ri(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VRSHRsIMM.

unsigned fastEmit_ARMISD_VRSHRsIMM_MVT_v8i8_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VRSHRsv8i8, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VRSHRsIMM_MVT_v16i8_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VRSHRsv16i8, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VRSHRsIMM_MVT_v4i16_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VRSHRsv4i16, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VRSHRsIMM_MVT_v8i16_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VRSHRsv8i16, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VRSHRsIMM_MVT_v2i32_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VRSHRsv2i32, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VRSHRsIMM_MVT_v4i32_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VRSHRsv4i32, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VRSHRsIMM_MVT_v1i64_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VRSHRsv1i64, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VRSHRsIMM_MVT_v2i64_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VRSHRsv2i64, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VRSHRsIMM_ri(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ARMISD_VRSHRsIMM_MVT_v8i8_ri(RetVT, Op0, imm1);
  case MVT::v16i8: return fastEmit_ARMISD_VRSHRsIMM_MVT_v16i8_ri(RetVT, Op0, imm1);
  case MVT::v4i16: return fastEmit_ARMISD_VRSHRsIMM_MVT_v4i16_ri(RetVT, Op0, imm1);
  case MVT::v8i16: return fastEmit_ARMISD_VRSHRsIMM_MVT_v8i16_ri(RetVT, Op0, imm1);
  case MVT::v2i32: return fastEmit_ARMISD_VRSHRsIMM_MVT_v2i32_ri(RetVT, Op0, imm1);
  case MVT::v4i32: return fastEmit_ARMISD_VRSHRsIMM_MVT_v4i32_ri(RetVT, Op0, imm1);
  case MVT::v1i64: return fastEmit_ARMISD_VRSHRsIMM_MVT_v1i64_ri(RetVT, Op0, imm1);
  case MVT::v2i64: return fastEmit_ARMISD_VRSHRsIMM_MVT_v2i64_ri(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VRSHRuIMM.

unsigned fastEmit_ARMISD_VRSHRuIMM_MVT_v8i8_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VRSHRuv8i8, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VRSHRuIMM_MVT_v16i8_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VRSHRuv16i8, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VRSHRuIMM_MVT_v4i16_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VRSHRuv4i16, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VRSHRuIMM_MVT_v8i16_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VRSHRuv8i16, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VRSHRuIMM_MVT_v2i32_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VRSHRuv2i32, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VRSHRuIMM_MVT_v4i32_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VRSHRuv4i32, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VRSHRuIMM_MVT_v1i64_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VRSHRuv1i64, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VRSHRuIMM_MVT_v2i64_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VRSHRuv2i64, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VRSHRuIMM_ri(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ARMISD_VRSHRuIMM_MVT_v8i8_ri(RetVT, Op0, imm1);
  case MVT::v16i8: return fastEmit_ARMISD_VRSHRuIMM_MVT_v16i8_ri(RetVT, Op0, imm1);
  case MVT::v4i16: return fastEmit_ARMISD_VRSHRuIMM_MVT_v4i16_ri(RetVT, Op0, imm1);
  case MVT::v8i16: return fastEmit_ARMISD_VRSHRuIMM_MVT_v8i16_ri(RetVT, Op0, imm1);
  case MVT::v2i32: return fastEmit_ARMISD_VRSHRuIMM_MVT_v2i32_ri(RetVT, Op0, imm1);
  case MVT::v4i32: return fastEmit_ARMISD_VRSHRuIMM_MVT_v4i32_ri(RetVT, Op0, imm1);
  case MVT::v1i64: return fastEmit_ARMISD_VRSHRuIMM_MVT_v1i64_ri(RetVT, Op0, imm1);
  case MVT::v2i64: return fastEmit_ARMISD_VRSHRuIMM_MVT_v2i64_ri(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VSHLIMM.

unsigned fastEmit_ARMISD_VSHLIMM_MVT_v8i8_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VSHLiv8i8, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHLIMM_MVT_v16i8_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VSHLiv16i8, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHLIMM_MVT_v4i16_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VSHLiv4i16, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHLIMM_MVT_v8i16_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VSHLiv8i16, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHLIMM_MVT_v2i32_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VSHLiv2i32, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHLIMM_MVT_v4i32_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VSHLiv4i32, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHLIMM_MVT_v1i64_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VSHLiv1i64, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHLIMM_MVT_v2i64_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VSHLiv2i64, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHLIMM_ri(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ARMISD_VSHLIMM_MVT_v8i8_ri(RetVT, Op0, imm1);
  case MVT::v16i8: return fastEmit_ARMISD_VSHLIMM_MVT_v16i8_ri(RetVT, Op0, imm1);
  case MVT::v4i16: return fastEmit_ARMISD_VSHLIMM_MVT_v4i16_ri(RetVT, Op0, imm1);
  case MVT::v8i16: return fastEmit_ARMISD_VSHLIMM_MVT_v8i16_ri(RetVT, Op0, imm1);
  case MVT::v2i32: return fastEmit_ARMISD_VSHLIMM_MVT_v2i32_ri(RetVT, Op0, imm1);
  case MVT::v4i32: return fastEmit_ARMISD_VSHLIMM_MVT_v4i32_ri(RetVT, Op0, imm1);
  case MVT::v1i64: return fastEmit_ARMISD_VSHLIMM_MVT_v1i64_ri(RetVT, Op0, imm1);
  case MVT::v2i64: return fastEmit_ARMISD_VSHLIMM_MVT_v2i64_ri(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VSHRsIMM.

unsigned fastEmit_ARMISD_VSHRsIMM_MVT_v8i8_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VSHRsv8i8, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHRsIMM_MVT_v16i8_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VSHRsv16i8, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHRsIMM_MVT_v4i16_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VSHRsv4i16, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHRsIMM_MVT_v8i16_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VSHRsv8i16, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHRsIMM_MVT_v2i32_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VSHRsv2i32, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHRsIMM_MVT_v4i32_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VSHRsv4i32, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHRsIMM_MVT_v1i64_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VSHRsv1i64, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHRsIMM_MVT_v2i64_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VSHRsv2i64, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHRsIMM_ri(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ARMISD_VSHRsIMM_MVT_v8i8_ri(RetVT, Op0, imm1);
  case MVT::v16i8: return fastEmit_ARMISD_VSHRsIMM_MVT_v16i8_ri(RetVT, Op0, imm1);
  case MVT::v4i16: return fastEmit_ARMISD_VSHRsIMM_MVT_v4i16_ri(RetVT, Op0, imm1);
  case MVT::v8i16: return fastEmit_ARMISD_VSHRsIMM_MVT_v8i16_ri(RetVT, Op0, imm1);
  case MVT::v2i32: return fastEmit_ARMISD_VSHRsIMM_MVT_v2i32_ri(RetVT, Op0, imm1);
  case MVT::v4i32: return fastEmit_ARMISD_VSHRsIMM_MVT_v4i32_ri(RetVT, Op0, imm1);
  case MVT::v1i64: return fastEmit_ARMISD_VSHRsIMM_MVT_v1i64_ri(RetVT, Op0, imm1);
  case MVT::v2i64: return fastEmit_ARMISD_VSHRsIMM_MVT_v2i64_ri(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VSHRuIMM.

unsigned fastEmit_ARMISD_VSHRuIMM_MVT_v8i8_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VSHRuv8i8, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHRuIMM_MVT_v16i8_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VSHRuv16i8, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHRuIMM_MVT_v4i16_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VSHRuv4i16, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHRuIMM_MVT_v8i16_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VSHRuv8i16, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHRuIMM_MVT_v2i32_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VSHRuv2i32, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHRuIMM_MVT_v4i32_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VSHRuv4i32, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHRuIMM_MVT_v1i64_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VSHRuv1i64, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHRuIMM_MVT_v2i64_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VSHRuv2i64, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHRuIMM_ri(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ARMISD_VSHRuIMM_MVT_v8i8_ri(RetVT, Op0, imm1);
  case MVT::v16i8: return fastEmit_ARMISD_VSHRuIMM_MVT_v16i8_ri(RetVT, Op0, imm1);
  case MVT::v4i16: return fastEmit_ARMISD_VSHRuIMM_MVT_v4i16_ri(RetVT, Op0, imm1);
  case MVT::v8i16: return fastEmit_ARMISD_VSHRuIMM_MVT_v8i16_ri(RetVT, Op0, imm1);
  case MVT::v2i32: return fastEmit_ARMISD_VSHRuIMM_MVT_v2i32_ri(RetVT, Op0, imm1);
  case MVT::v4i32: return fastEmit_ARMISD_VSHRuIMM_MVT_v4i32_ri(RetVT, Op0, imm1);
  case MVT::v1i64: return fastEmit_ARMISD_VSHRuIMM_MVT_v1i64_ri(RetVT, Op0, imm1);
  case MVT::v2i64: return fastEmit_ARMISD_VSHRuIMM_MVT_v2i64_ri(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::EXTRACT_VECTOR_ELT.

unsigned fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v2i32_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasFPRegs()) && (!Subtarget->hasSlowVGETLNi32())) {
    return fastEmitInst_ri(ARM::VGETLNi32, &ARM::GPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_EXTRACT_VECTOR_ELT_ri(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v2i32: return fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v2i32_ri(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SHL.

unsigned fastEmit_ISD_SHL_MVT_i32_ri(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb()) && (Subtarget->isThumb1Only())) {
    return fastEmitInst_ri(ARM::tLSLri, &ARM::tGPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_SHL_ri(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SHL_MVT_i32_ri(RetVT, Op0, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, uint64_t imm1) override {
  if (VT == MVT::i32 && Predicate_mod_imm(imm1))
    if (unsigned Reg = fastEmit_ri_Predicate_mod_imm(VT, RetVT, Opcode, Op0, imm1))
      return Reg;

  if (VT == MVT::i32 && Predicate_imm0_7(imm1))
    if (unsigned Reg = fastEmit_ri_Predicate_imm0_7(VT, RetVT, Opcode, Op0, imm1))
      return Reg;

  if (VT == MVT::i32 && Predicate_imm8_255(imm1))
    if (unsigned Reg = fastEmit_ri_Predicate_imm8_255(VT, RetVT, Opcode, Op0, imm1))
      return Reg;

  if (VT == MVT::i32 && Predicate_imm0_255(imm1))
    if (unsigned Reg = fastEmit_ri_Predicate_imm0_255(VT, RetVT, Opcode, Op0, imm1))
      return Reg;

  if (VT == MVT::i32 && Predicate_t2_so_imm(imm1))
    if (unsigned Reg = fastEmit_ri_Predicate_t2_so_imm(VT, RetVT, Opcode, Op0, imm1))
      return Reg;

  if (VT == MVT::i32 && Predicate_imm0_4095(imm1))
    if (unsigned Reg = fastEmit_ri_Predicate_imm0_4095(VT, RetVT, Opcode, Op0, imm1))
      return Reg;

  if (VT == MVT::i32 && Predicate_imm1_31(imm1))
    if (unsigned Reg = fastEmit_ri_Predicate_imm1_31(VT, RetVT, Opcode, Op0, imm1))
      return Reg;

  if (VT == MVT::i32 && Predicate_shr_imm8(imm1))
    if (unsigned Reg = fastEmit_ri_Predicate_shr_imm8(VT, RetVT, Opcode, Op0, imm1))
      return Reg;

  if (VT == MVT::i32 && Predicate_shr_imm16(imm1))
    if (unsigned Reg = fastEmit_ri_Predicate_shr_imm16(VT, RetVT, Opcode, Op0, imm1))
      return Reg;

  if (VT == MVT::i32 && Predicate_shr_imm32(imm1))
    if (unsigned Reg = fastEmit_ri_Predicate_shr_imm32(VT, RetVT, Opcode, Op0, imm1))
      return Reg;

  if (VT == MVT::i32 && Predicate_VectorIndex32(imm1))
    if (unsigned Reg = fastEmit_ri_Predicate_VectorIndex32(VT, RetVT, Opcode, Op0, imm1))
      return Reg;

  if (VT == MVT::i32 && Predicate_imm0_31(imm1))
    if (unsigned Reg = fastEmit_ri_Predicate_imm0_31(VT, RetVT, Opcode, Op0, imm1))
      return Reg;

  if (VT == MVT::i32 && Predicate_imm0_15(imm1))
    if (unsigned Reg = fastEmit_ri_Predicate_imm0_15(VT, RetVT, Opcode, Op0, imm1))
      return Reg;

  switch (Opcode) {
  case ARMISD::PIC_ADD: return fastEmit_ARMISD_PIC_ADD_ri(VT, RetVT, Op0, imm1);
  case ARMISD::VDUPLANE: return fastEmit_ARMISD_VDUPLANE_ri(VT, RetVT, Op0, imm1);
  case ARMISD::VGETLANEs: return fastEmit_ARMISD_VGETLANEs_ri(VT, RetVT, Op0, imm1);
  case ARMISD::VGETLANEu: return fastEmit_ARMISD_VGETLANEu_ri(VT, RetVT, Op0, imm1);
  case ARMISD::VQSHLsIMM: return fastEmit_ARMISD_VQSHLsIMM_ri(VT, RetVT, Op0, imm1);
  case ARMISD::VQSHLsuIMM: return fastEmit_ARMISD_VQSHLsuIMM_ri(VT, RetVT, Op0, imm1);
  case ARMISD::VQSHLuIMM: return fastEmit_ARMISD_VQSHLuIMM_ri(VT, RetVT, Op0, imm1);
  case ARMISD::VRSHRsIMM: return fastEmit_ARMISD_VRSHRsIMM_ri(VT, RetVT, Op0, imm1);
  case ARMISD::VRSHRuIMM: return fastEmit_ARMISD_VRSHRuIMM_ri(VT, RetVT, Op0, imm1);
  case ARMISD::VSHLIMM: return fastEmit_ARMISD_VSHLIMM_ri(VT, RetVT, Op0, imm1);
  case ARMISD::VSHRsIMM: return fastEmit_ARMISD_VSHRsIMM_ri(VT, RetVT, Op0, imm1);
  case ARMISD::VSHRuIMM: return fastEmit_ARMISD_VSHRuIMM_ri(VT, RetVT, Op0, imm1);
  case ISD::EXTRACT_VECTOR_ELT: return fastEmit_ISD_EXTRACT_VECTOR_ELT_ri(VT, RetVT, Op0, imm1);
  case ISD::SHL: return fastEmit_ISD_SHL_ri(VT, RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::CMN.

unsigned fastEmit_ARMISD_CMN_MVT_i32_ri_Predicate_mod_imm(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((!Subtarget->isThumb())) {
    return fastEmitInst_ri(ARM::CMNri, &ARM::GPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_CMN_ri_Predicate_mod_imm(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_CMN_MVT_i32_ri_Predicate_mod_imm(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::CMP.

unsigned fastEmit_ARMISD_CMP_MVT_i32_ri_Predicate_mod_imm(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((!Subtarget->isThumb())) {
    return fastEmitInst_ri(ARM::CMPri, &ARM::GPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_CMP_ri_Predicate_mod_imm(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_CMP_MVT_i32_ri_Predicate_mod_imm(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::CMPZ.

unsigned fastEmit_ARMISD_CMPZ_MVT_i32_ri_Predicate_mod_imm(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((!Subtarget->isThumb())) {
    return fastEmitInst_ri(ARM::CMPri, &ARM::GPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_CMPZ_ri_Predicate_mod_imm(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_CMPZ_MVT_i32_ri_Predicate_mod_imm(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::SUBS.

unsigned fastEmit_ARMISD_SUBS_MVT_i32_ri_Predicate_mod_imm(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((!Subtarget->isThumb())) {
    return fastEmitInst_ri(ARM::SUBSri, &ARM::GPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_SUBS_ri_Predicate_mod_imm(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_SUBS_MVT_i32_ri_Predicate_mod_imm(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::ADD.

unsigned fastEmit_ISD_ADD_MVT_i32_ri_Predicate_mod_imm(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((!Subtarget->isThumb())) {
    return fastEmitInst_ri(ARM::ADDri, &ARM::GPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_ri_Predicate_mod_imm(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_ADD_MVT_i32_ri_Predicate_mod_imm(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::AND.

unsigned fastEmit_ISD_AND_MVT_i32_ri_Predicate_mod_imm(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((!Subtarget->isThumb())) {
    return fastEmitInst_ri(ARM::ANDri, &ARM::GPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_ri_Predicate_mod_imm(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_AND_MVT_i32_ri_Predicate_mod_imm(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::OR.

unsigned fastEmit_ISD_OR_MVT_i32_ri_Predicate_mod_imm(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((!Subtarget->isThumb())) {
    return fastEmitInst_ri(ARM::ORRri, &ARM::GPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_ri_Predicate_mod_imm(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_OR_MVT_i32_ri_Predicate_mod_imm(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SUB.

unsigned fastEmit_ISD_SUB_MVT_i32_ri_Predicate_mod_imm(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((!Subtarget->isThumb())) {
    return fastEmitInst_ri(ARM::SUBri, &ARM::GPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_ri_Predicate_mod_imm(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SUB_MVT_i32_ri_Predicate_mod_imm(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::XOR.

unsigned fastEmit_ISD_XOR_MVT_i32_ri_Predicate_mod_imm(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((!Subtarget->isThumb())) {
    return fastEmitInst_ri(ARM::EORri, &ARM::GPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_ri_Predicate_mod_imm(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_XOR_MVT_i32_ri_Predicate_mod_imm(RetVT, Op0, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_mod_imm(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, uint64_t imm1) {
  switch (Opcode) {
  case ARMISD::CMN: return fastEmit_ARMISD_CMN_ri_Predicate_mod_imm(VT, RetVT, Op0, imm1);
  case ARMISD::CMP: return fastEmit_ARMISD_CMP_ri_Predicate_mod_imm(VT, RetVT, Op0, imm1);
  case ARMISD::CMPZ: return fastEmit_ARMISD_CMPZ_ri_Predicate_mod_imm(VT, RetVT, Op0, imm1);
  case ARMISD::SUBS: return fastEmit_ARMISD_SUBS_ri_Predicate_mod_imm(VT, RetVT, Op0, imm1);
  case ISD::ADD: return fastEmit_ISD_ADD_ri_Predicate_mod_imm(VT, RetVT, Op0, imm1);
  case ISD::AND: return fastEmit_ISD_AND_ri_Predicate_mod_imm(VT, RetVT, Op0, imm1);
  case ISD::OR: return fastEmit_ISD_OR_ri_Predicate_mod_imm(VT, RetVT, Op0, imm1);
  case ISD::SUB: return fastEmit_ISD_SUB_ri_Predicate_mod_imm(VT, RetVT, Op0, imm1);
  case ISD::XOR: return fastEmit_ISD_XOR_ri_Predicate_mod_imm(VT, RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::SUBS.

unsigned fastEmit_ARMISD_SUBS_MVT_i32_ri_Predicate_imm0_7(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb()) && (Subtarget->isThumb1Only())) {
    return fastEmitInst_ri(ARM::tSUBSi3, &ARM::tGPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_SUBS_ri_Predicate_imm0_7(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_SUBS_MVT_i32_ri_Predicate_imm0_7(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VSHLIMM.

unsigned fastEmit_ARMISD_VSHLIMM_MVT_v16i8_ri_Predicate_imm0_7(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_ri(ARM::MVE_VSHL_immi8, &ARM::MQPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHLIMM_ri_Predicate_imm0_7(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ARMISD_VSHLIMM_MVT_v16i8_ri_Predicate_imm0_7(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VSHRsIMM.

unsigned fastEmit_ARMISD_VSHRsIMM_MVT_v16i8_ri_Predicate_imm0_7(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_ri(ARM::MVE_VSHR_imms8, &ARM::MQPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHRsIMM_ri_Predicate_imm0_7(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ARMISD_VSHRsIMM_MVT_v16i8_ri_Predicate_imm0_7(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VSHRuIMM.

unsigned fastEmit_ARMISD_VSHRuIMM_MVT_v16i8_ri_Predicate_imm0_7(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_ri(ARM::MVE_VSHR_immu8, &ARM::MQPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHRuIMM_ri_Predicate_imm0_7(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ARMISD_VSHRuIMM_MVT_v16i8_ri_Predicate_imm0_7(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::ADD.

unsigned fastEmit_ISD_ADD_MVT_i32_ri_Predicate_imm0_7(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb()) && (Subtarget->isThumb1Only())) {
    return fastEmitInst_ri(ARM::tADDi3, &ARM::tGPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_ri_Predicate_imm0_7(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_ADD_MVT_i32_ri_Predicate_imm0_7(RetVT, Op0, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_imm0_7(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, uint64_t imm1) {
  switch (Opcode) {
  case ARMISD::SUBS: return fastEmit_ARMISD_SUBS_ri_Predicate_imm0_7(VT, RetVT, Op0, imm1);
  case ARMISD::VSHLIMM: return fastEmit_ARMISD_VSHLIMM_ri_Predicate_imm0_7(VT, RetVT, Op0, imm1);
  case ARMISD::VSHRsIMM: return fastEmit_ARMISD_VSHRsIMM_ri_Predicate_imm0_7(VT, RetVT, Op0, imm1);
  case ARMISD::VSHRuIMM: return fastEmit_ARMISD_VSHRuIMM_ri_Predicate_imm0_7(VT, RetVT, Op0, imm1);
  case ISD::ADD: return fastEmit_ISD_ADD_ri_Predicate_imm0_7(VT, RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::ADD.

unsigned fastEmit_ISD_ADD_MVT_i32_ri_Predicate_imm8_255(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb()) && (Subtarget->isThumb1Only())) {
    return fastEmitInst_ri(ARM::tADDi8, &ARM::tGPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_ri_Predicate_imm8_255(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_ADD_MVT_i32_ri_Predicate_imm8_255(RetVT, Op0, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_imm8_255(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, uint64_t imm1) {
  switch (Opcode) {
  case ISD::ADD: return fastEmit_ISD_ADD_ri_Predicate_imm8_255(VT, RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::CMP.

unsigned fastEmit_ARMISD_CMP_MVT_i32_ri_Predicate_imm0_255(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->isThumb()) && (Subtarget->isThumb1Only())) {
    return fastEmitInst_ri(ARM::tCMPi8, &ARM::tGPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_CMP_ri_Predicate_imm0_255(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_CMP_MVT_i32_ri_Predicate_imm0_255(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::CMPZ.

unsigned fastEmit_ARMISD_CMPZ_MVT_i32_ri_Predicate_imm0_255(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->isThumb()) && (Subtarget->isThumb1Only())) {
    return fastEmitInst_ri(ARM::tCMPi8, &ARM::tGPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_CMPZ_ri_Predicate_imm0_255(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_CMPZ_MVT_i32_ri_Predicate_imm0_255(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::SUBS.

unsigned fastEmit_ARMISD_SUBS_MVT_i32_ri_Predicate_imm0_255(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb()) && (Subtarget->isThumb1Only())) {
    return fastEmitInst_ri(ARM::tSUBSi8, &ARM::tGPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_SUBS_ri_Predicate_imm0_255(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_SUBS_MVT_i32_ri_Predicate_imm0_255(RetVT, Op0, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_imm0_255(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, uint64_t imm1) {
  switch (Opcode) {
  case ARMISD::CMP: return fastEmit_ARMISD_CMP_ri_Predicate_imm0_255(VT, RetVT, Op0, imm1);
  case ARMISD::CMPZ: return fastEmit_ARMISD_CMPZ_ri_Predicate_imm0_255(VT, RetVT, Op0, imm1);
  case ARMISD::SUBS: return fastEmit_ARMISD_SUBS_ri_Predicate_imm0_255(VT, RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::CMP.

unsigned fastEmit_ARMISD_CMP_MVT_i32_ri_Predicate_t2_so_imm(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->isThumb2())) {
    return fastEmitInst_ri(ARM::t2CMPri, &ARM::GPRnopcRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_CMP_ri_Predicate_t2_so_imm(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_CMP_MVT_i32_ri_Predicate_t2_so_imm(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::CMPZ.

unsigned fastEmit_ARMISD_CMPZ_MVT_i32_ri_Predicate_t2_so_imm(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->isThumb2())) {
    return fastEmitInst_ri(ARM::t2CMPri, &ARM::GPRnopcRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_CMPZ_ri_Predicate_t2_so_imm(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_CMPZ_MVT_i32_ri_Predicate_t2_so_imm(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::SUBS.

unsigned fastEmit_ARMISD_SUBS_MVT_i32_ri_Predicate_t2_so_imm(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb2())) {
    return fastEmitInst_ri(ARM::t2SUBSri, &ARM::rGPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_SUBS_ri_Predicate_t2_so_imm(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ARMISD_SUBS_MVT_i32_ri_Predicate_t2_so_imm(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::ADD.

unsigned fastEmit_ISD_ADD_MVT_i32_ri_Predicate_t2_so_imm(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb2())) {
    return fastEmitInst_ri(ARM::t2ADDri, &ARM::rGPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_ri_Predicate_t2_so_imm(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_ADD_MVT_i32_ri_Predicate_t2_so_imm(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::AND.

unsigned fastEmit_ISD_AND_MVT_i32_ri_Predicate_t2_so_imm(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb2())) {
    return fastEmitInst_ri(ARM::t2ANDri, &ARM::rGPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_ri_Predicate_t2_so_imm(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_AND_MVT_i32_ri_Predicate_t2_so_imm(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::OR.

unsigned fastEmit_ISD_OR_MVT_i32_ri_Predicate_t2_so_imm(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb2())) {
    return fastEmitInst_ri(ARM::t2ORRri, &ARM::rGPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_ri_Predicate_t2_so_imm(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_OR_MVT_i32_ri_Predicate_t2_so_imm(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SUB.

unsigned fastEmit_ISD_SUB_MVT_i32_ri_Predicate_t2_so_imm(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb2())) {
    return fastEmitInst_ri(ARM::t2SUBri, &ARM::rGPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_ri_Predicate_t2_so_imm(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SUB_MVT_i32_ri_Predicate_t2_so_imm(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::XOR.

unsigned fastEmit_ISD_XOR_MVT_i32_ri_Predicate_t2_so_imm(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb2())) {
    return fastEmitInst_ri(ARM::t2EORri, &ARM::rGPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_ri_Predicate_t2_so_imm(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_XOR_MVT_i32_ri_Predicate_t2_so_imm(RetVT, Op0, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_t2_so_imm(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, uint64_t imm1) {
  switch (Opcode) {
  case ARMISD::CMP: return fastEmit_ARMISD_CMP_ri_Predicate_t2_so_imm(VT, RetVT, Op0, imm1);
  case ARMISD::CMPZ: return fastEmit_ARMISD_CMPZ_ri_Predicate_t2_so_imm(VT, RetVT, Op0, imm1);
  case ARMISD::SUBS: return fastEmit_ARMISD_SUBS_ri_Predicate_t2_so_imm(VT, RetVT, Op0, imm1);
  case ISD::ADD: return fastEmit_ISD_ADD_ri_Predicate_t2_so_imm(VT, RetVT, Op0, imm1);
  case ISD::AND: return fastEmit_ISD_AND_ri_Predicate_t2_so_imm(VT, RetVT, Op0, imm1);
  case ISD::OR: return fastEmit_ISD_OR_ri_Predicate_t2_so_imm(VT, RetVT, Op0, imm1);
  case ISD::SUB: return fastEmit_ISD_SUB_ri_Predicate_t2_so_imm(VT, RetVT, Op0, imm1);
  case ISD::XOR: return fastEmit_ISD_XOR_ri_Predicate_t2_so_imm(VT, RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::ADD.

unsigned fastEmit_ISD_ADD_MVT_i32_ri_Predicate_imm0_4095(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb2())) {
    return fastEmitInst_ri(ARM::t2ADDri12, &ARM::rGPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_ri_Predicate_imm0_4095(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_ADD_MVT_i32_ri_Predicate_imm0_4095(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SUB.

unsigned fastEmit_ISD_SUB_MVT_i32_ri_Predicate_imm0_4095(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb2())) {
    return fastEmitInst_ri(ARM::t2SUBri12, &ARM::rGPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_ri_Predicate_imm0_4095(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SUB_MVT_i32_ri_Predicate_imm0_4095(RetVT, Op0, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_imm0_4095(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, uint64_t imm1) {
  switch (Opcode) {
  case ISD::ADD: return fastEmit_ISD_ADD_ri_Predicate_imm0_4095(VT, RetVT, Op0, imm1);
  case ISD::SUB: return fastEmit_ISD_SUB_ri_Predicate_imm0_4095(VT, RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::ROTR.

unsigned fastEmit_ISD_ROTR_MVT_i32_ri_Predicate_imm1_31(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb2())) {
    return fastEmitInst_ri(ARM::t2RORri, &ARM::rGPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_ROTR_ri_Predicate_imm1_31(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_ROTR_MVT_i32_ri_Predicate_imm1_31(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SHL.

unsigned fastEmit_ISD_SHL_MVT_i32_ri_Predicate_imm1_31(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb2())) {
    return fastEmitInst_ri(ARM::t2LSLri, &ARM::rGPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_SHL_ri_Predicate_imm1_31(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SHL_MVT_i32_ri_Predicate_imm1_31(RetVT, Op0, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_imm1_31(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, uint64_t imm1) {
  switch (Opcode) {
  case ISD::ROTR: return fastEmit_ISD_ROTR_ri_Predicate_imm1_31(VT, RetVT, Op0, imm1);
  case ISD::SHL: return fastEmit_ISD_SHL_ri_Predicate_imm1_31(VT, RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VQRSHRNsIMM.

unsigned fastEmit_ARMISD_VQRSHRNsIMM_MVT_v8i16_ri_Predicate_shr_imm8(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQRSHRNsv8i8, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQRSHRNsIMM_ri_Predicate_shr_imm8(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_ARMISD_VQRSHRNsIMM_MVT_v8i16_ri_Predicate_shr_imm8(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VQRSHRNsuIMM.

unsigned fastEmit_ARMISD_VQRSHRNsuIMM_MVT_v8i16_ri_Predicate_shr_imm8(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQRSHRUNv8i8, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQRSHRNsuIMM_ri_Predicate_shr_imm8(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_ARMISD_VQRSHRNsuIMM_MVT_v8i16_ri_Predicate_shr_imm8(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VQRSHRNuIMM.

unsigned fastEmit_ARMISD_VQRSHRNuIMM_MVT_v8i16_ri_Predicate_shr_imm8(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQRSHRNuv8i8, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQRSHRNuIMM_ri_Predicate_shr_imm8(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_ARMISD_VQRSHRNuIMM_MVT_v8i16_ri_Predicate_shr_imm8(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VQSHRNsIMM.

unsigned fastEmit_ARMISD_VQSHRNsIMM_MVT_v8i16_ri_Predicate_shr_imm8(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHRNsv8i8, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHRNsIMM_ri_Predicate_shr_imm8(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_ARMISD_VQSHRNsIMM_MVT_v8i16_ri_Predicate_shr_imm8(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VQSHRNsuIMM.

unsigned fastEmit_ARMISD_VQSHRNsuIMM_MVT_v8i16_ri_Predicate_shr_imm8(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHRUNv8i8, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHRNsuIMM_ri_Predicate_shr_imm8(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_ARMISD_VQSHRNsuIMM_MVT_v8i16_ri_Predicate_shr_imm8(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VQSHRNuIMM.

unsigned fastEmit_ARMISD_VQSHRNuIMM_MVT_v8i16_ri_Predicate_shr_imm8(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHRNuv8i8, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHRNuIMM_ri_Predicate_shr_imm8(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_ARMISD_VQSHRNuIMM_MVT_v8i16_ri_Predicate_shr_imm8(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VRSHRNIMM.

unsigned fastEmit_ARMISD_VRSHRNIMM_MVT_v8i16_ri_Predicate_shr_imm8(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VRSHRNv8i8, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VRSHRNIMM_ri_Predicate_shr_imm8(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_ARMISD_VRSHRNIMM_MVT_v8i16_ri_Predicate_shr_imm8(RetVT, Op0, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_shr_imm8(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, uint64_t imm1) {
  switch (Opcode) {
  case ARMISD::VQRSHRNsIMM: return fastEmit_ARMISD_VQRSHRNsIMM_ri_Predicate_shr_imm8(VT, RetVT, Op0, imm1);
  case ARMISD::VQRSHRNsuIMM: return fastEmit_ARMISD_VQRSHRNsuIMM_ri_Predicate_shr_imm8(VT, RetVT, Op0, imm1);
  case ARMISD::VQRSHRNuIMM: return fastEmit_ARMISD_VQRSHRNuIMM_ri_Predicate_shr_imm8(VT, RetVT, Op0, imm1);
  case ARMISD::VQSHRNsIMM: return fastEmit_ARMISD_VQSHRNsIMM_ri_Predicate_shr_imm8(VT, RetVT, Op0, imm1);
  case ARMISD::VQSHRNsuIMM: return fastEmit_ARMISD_VQSHRNsuIMM_ri_Predicate_shr_imm8(VT, RetVT, Op0, imm1);
  case ARMISD::VQSHRNuIMM: return fastEmit_ARMISD_VQSHRNuIMM_ri_Predicate_shr_imm8(VT, RetVT, Op0, imm1);
  case ARMISD::VRSHRNIMM: return fastEmit_ARMISD_VRSHRNIMM_ri_Predicate_shr_imm8(VT, RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VQRSHRNsIMM.

unsigned fastEmit_ARMISD_VQRSHRNsIMM_MVT_v4i32_ri_Predicate_shr_imm16(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQRSHRNsv4i16, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQRSHRNsIMM_ri_Predicate_shr_imm16(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v4i32: return fastEmit_ARMISD_VQRSHRNsIMM_MVT_v4i32_ri_Predicate_shr_imm16(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VQRSHRNsuIMM.

unsigned fastEmit_ARMISD_VQRSHRNsuIMM_MVT_v4i32_ri_Predicate_shr_imm16(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQRSHRUNv4i16, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQRSHRNsuIMM_ri_Predicate_shr_imm16(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v4i32: return fastEmit_ARMISD_VQRSHRNsuIMM_MVT_v4i32_ri_Predicate_shr_imm16(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VQRSHRNuIMM.

unsigned fastEmit_ARMISD_VQRSHRNuIMM_MVT_v4i32_ri_Predicate_shr_imm16(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQRSHRNuv4i16, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQRSHRNuIMM_ri_Predicate_shr_imm16(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v4i32: return fastEmit_ARMISD_VQRSHRNuIMM_MVT_v4i32_ri_Predicate_shr_imm16(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VQSHRNsIMM.

unsigned fastEmit_ARMISD_VQSHRNsIMM_MVT_v4i32_ri_Predicate_shr_imm16(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHRNsv4i16, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHRNsIMM_ri_Predicate_shr_imm16(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v4i32: return fastEmit_ARMISD_VQSHRNsIMM_MVT_v4i32_ri_Predicate_shr_imm16(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VQSHRNsuIMM.

unsigned fastEmit_ARMISD_VQSHRNsuIMM_MVT_v4i32_ri_Predicate_shr_imm16(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHRUNv4i16, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHRNsuIMM_ri_Predicate_shr_imm16(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v4i32: return fastEmit_ARMISD_VQSHRNsuIMM_MVT_v4i32_ri_Predicate_shr_imm16(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VQSHRNuIMM.

unsigned fastEmit_ARMISD_VQSHRNuIMM_MVT_v4i32_ri_Predicate_shr_imm16(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHRNuv4i16, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHRNuIMM_ri_Predicate_shr_imm16(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v4i32: return fastEmit_ARMISD_VQSHRNuIMM_MVT_v4i32_ri_Predicate_shr_imm16(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VRSHRNIMM.

unsigned fastEmit_ARMISD_VRSHRNIMM_MVT_v4i32_ri_Predicate_shr_imm16(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VRSHRNv4i16, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VRSHRNIMM_ri_Predicate_shr_imm16(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v4i32: return fastEmit_ARMISD_VRSHRNIMM_MVT_v4i32_ri_Predicate_shr_imm16(RetVT, Op0, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_shr_imm16(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, uint64_t imm1) {
  switch (Opcode) {
  case ARMISD::VQRSHRNsIMM: return fastEmit_ARMISD_VQRSHRNsIMM_ri_Predicate_shr_imm16(VT, RetVT, Op0, imm1);
  case ARMISD::VQRSHRNsuIMM: return fastEmit_ARMISD_VQRSHRNsuIMM_ri_Predicate_shr_imm16(VT, RetVT, Op0, imm1);
  case ARMISD::VQRSHRNuIMM: return fastEmit_ARMISD_VQRSHRNuIMM_ri_Predicate_shr_imm16(VT, RetVT, Op0, imm1);
  case ARMISD::VQSHRNsIMM: return fastEmit_ARMISD_VQSHRNsIMM_ri_Predicate_shr_imm16(VT, RetVT, Op0, imm1);
  case ARMISD::VQSHRNsuIMM: return fastEmit_ARMISD_VQSHRNsuIMM_ri_Predicate_shr_imm16(VT, RetVT, Op0, imm1);
  case ARMISD::VQSHRNuIMM: return fastEmit_ARMISD_VQSHRNuIMM_ri_Predicate_shr_imm16(VT, RetVT, Op0, imm1);
  case ARMISD::VRSHRNIMM: return fastEmit_ARMISD_VRSHRNIMM_ri_Predicate_shr_imm16(VT, RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VQRSHRNsIMM.

unsigned fastEmit_ARMISD_VQRSHRNsIMM_MVT_v2i64_ri_Predicate_shr_imm32(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQRSHRNsv2i32, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQRSHRNsIMM_ri_Predicate_shr_imm32(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v2i64: return fastEmit_ARMISD_VQRSHRNsIMM_MVT_v2i64_ri_Predicate_shr_imm32(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VQRSHRNsuIMM.

unsigned fastEmit_ARMISD_VQRSHRNsuIMM_MVT_v2i64_ri_Predicate_shr_imm32(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQRSHRUNv2i32, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQRSHRNsuIMM_ri_Predicate_shr_imm32(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v2i64: return fastEmit_ARMISD_VQRSHRNsuIMM_MVT_v2i64_ri_Predicate_shr_imm32(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VQRSHRNuIMM.

unsigned fastEmit_ARMISD_VQRSHRNuIMM_MVT_v2i64_ri_Predicate_shr_imm32(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQRSHRNuv2i32, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQRSHRNuIMM_ri_Predicate_shr_imm32(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v2i64: return fastEmit_ARMISD_VQRSHRNuIMM_MVT_v2i64_ri_Predicate_shr_imm32(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VQSHRNsIMM.

unsigned fastEmit_ARMISD_VQSHRNsIMM_MVT_v2i64_ri_Predicate_shr_imm32(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHRNsv2i32, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHRNsIMM_ri_Predicate_shr_imm32(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v2i64: return fastEmit_ARMISD_VQSHRNsIMM_MVT_v2i64_ri_Predicate_shr_imm32(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VQSHRNsuIMM.

unsigned fastEmit_ARMISD_VQSHRNsuIMM_MVT_v2i64_ri_Predicate_shr_imm32(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHRUNv2i32, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHRNsuIMM_ri_Predicate_shr_imm32(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v2i64: return fastEmit_ARMISD_VQSHRNsuIMM_MVT_v2i64_ri_Predicate_shr_imm32(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VQSHRNuIMM.

unsigned fastEmit_ARMISD_VQSHRNuIMM_MVT_v2i64_ri_Predicate_shr_imm32(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VQSHRNuv2i32, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VQSHRNuIMM_ri_Predicate_shr_imm32(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v2i64: return fastEmit_ARMISD_VQSHRNuIMM_MVT_v2i64_ri_Predicate_shr_imm32(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VRSHRNIMM.

unsigned fastEmit_ARMISD_VRSHRNIMM_MVT_v2i64_ri_Predicate_shr_imm32(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VRSHRNv2i32, &ARM::DPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VRSHRNIMM_ri_Predicate_shr_imm32(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v2i64: return fastEmit_ARMISD_VRSHRNIMM_MVT_v2i64_ri_Predicate_shr_imm32(RetVT, Op0, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_shr_imm32(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, uint64_t imm1) {
  switch (Opcode) {
  case ARMISD::VQRSHRNsIMM: return fastEmit_ARMISD_VQRSHRNsIMM_ri_Predicate_shr_imm32(VT, RetVT, Op0, imm1);
  case ARMISD::VQRSHRNsuIMM: return fastEmit_ARMISD_VQRSHRNsuIMM_ri_Predicate_shr_imm32(VT, RetVT, Op0, imm1);
  case ARMISD::VQRSHRNuIMM: return fastEmit_ARMISD_VQRSHRNuIMM_ri_Predicate_shr_imm32(VT, RetVT, Op0, imm1);
  case ARMISD::VQSHRNsIMM: return fastEmit_ARMISD_VQSHRNsIMM_ri_Predicate_shr_imm32(VT, RetVT, Op0, imm1);
  case ARMISD::VQSHRNsuIMM: return fastEmit_ARMISD_VQSHRNsuIMM_ri_Predicate_shr_imm32(VT, RetVT, Op0, imm1);
  case ARMISD::VQSHRNuIMM: return fastEmit_ARMISD_VQSHRNuIMM_ri_Predicate_shr_imm32(VT, RetVT, Op0, imm1);
  case ARMISD::VRSHRNIMM: return fastEmit_ARMISD_VRSHRNIMM_ri_Predicate_shr_imm32(VT, RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VDUPLANE.

unsigned fastEmit_ARMISD_VDUPLANE_MVT_v8i8_ri_Predicate_VectorIndex32(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VDUPLN8q, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VDUPLANE_MVT_v4i16_ri_Predicate_VectorIndex32(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VDUPLN16q, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VDUPLANE_MVT_v2i32_ri_Predicate_VectorIndex32(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(ARM::VDUPLN32q, &ARM::QPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VDUPLANE_ri_Predicate_VectorIndex32(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ARMISD_VDUPLANE_MVT_v8i8_ri_Predicate_VectorIndex32(RetVT, Op0, imm1);
  case MVT::v4i16: return fastEmit_ARMISD_VDUPLANE_MVT_v4i16_ri_Predicate_VectorIndex32(RetVT, Op0, imm1);
  case MVT::v2i32: return fastEmit_ARMISD_VDUPLANE_MVT_v2i32_ri_Predicate_VectorIndex32(RetVT, Op0, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_VectorIndex32(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, uint64_t imm1) {
  switch (Opcode) {
  case ARMISD::VDUPLANE: return fastEmit_ARMISD_VDUPLANE_ri_Predicate_VectorIndex32(VT, RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VSHLIMM.

unsigned fastEmit_ARMISD_VSHLIMM_MVT_v4i32_ri_Predicate_imm0_31(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_ri(ARM::MVE_VSHL_immi32, &ARM::MQPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHLIMM_ri_Predicate_imm0_31(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v4i32: return fastEmit_ARMISD_VSHLIMM_MVT_v4i32_ri_Predicate_imm0_31(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VSHRsIMM.

unsigned fastEmit_ARMISD_VSHRsIMM_MVT_v4i32_ri_Predicate_imm0_31(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_ri(ARM::MVE_VSHR_imms32, &ARM::MQPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHRsIMM_ri_Predicate_imm0_31(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v4i32: return fastEmit_ARMISD_VSHRsIMM_MVT_v4i32_ri_Predicate_imm0_31(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VSHRuIMM.

unsigned fastEmit_ARMISD_VSHRuIMM_MVT_v4i32_ri_Predicate_imm0_31(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_ri(ARM::MVE_VSHR_immu32, &ARM::MQPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHRuIMM_ri_Predicate_imm0_31(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v4i32: return fastEmit_ARMISD_VSHRuIMM_MVT_v4i32_ri_Predicate_imm0_31(RetVT, Op0, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_imm0_31(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, uint64_t imm1) {
  switch (Opcode) {
  case ARMISD::VSHLIMM: return fastEmit_ARMISD_VSHLIMM_ri_Predicate_imm0_31(VT, RetVT, Op0, imm1);
  case ARMISD::VSHRsIMM: return fastEmit_ARMISD_VSHRsIMM_ri_Predicate_imm0_31(VT, RetVT, Op0, imm1);
  case ARMISD::VSHRuIMM: return fastEmit_ARMISD_VSHRuIMM_ri_Predicate_imm0_31(VT, RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VSHLIMM.

unsigned fastEmit_ARMISD_VSHLIMM_MVT_v8i16_ri_Predicate_imm0_15(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_ri(ARM::MVE_VSHL_immi16, &ARM::MQPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHLIMM_ri_Predicate_imm0_15(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_ARMISD_VSHLIMM_MVT_v8i16_ri_Predicate_imm0_15(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VSHRsIMM.

unsigned fastEmit_ARMISD_VSHRsIMM_MVT_v8i16_ri_Predicate_imm0_15(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_ri(ARM::MVE_VSHR_imms16, &ARM::MQPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHRsIMM_ri_Predicate_imm0_15(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_ARMISD_VSHRsIMM_MVT_v8i16_ri_Predicate_imm0_15(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ARMISD::VSHRuIMM.

unsigned fastEmit_ARMISD_VSHRuIMM_MVT_v8i16_ri_Predicate_imm0_15(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMVEIntegerOps())) {
    return fastEmitInst_ri(ARM::MVE_VSHR_immu16, &ARM::MQPRRegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ARMISD_VSHRuIMM_ri_Predicate_imm0_15(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_ARMISD_VSHRuIMM_MVT_v8i16_ri_Predicate_imm0_15(RetVT, Op0, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_imm0_15(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, uint64_t imm1) {
  switch (Opcode) {
  case ARMISD::VSHLIMM: return fastEmit_ARMISD_VSHLIMM_ri_Predicate_imm0_15(VT, RetVT, Op0, imm1);
  case ARMISD::VSHRsIMM: return fastEmit_ARMISD_VSHRsIMM_ri_Predicate_imm0_15(VT, RetVT, Op0, imm1);
  case ARMISD::VSHRuIMM: return fastEmit_ARMISD_VSHRuIMM_ri_Predicate_imm0_15(VT, RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::Constant.

unsigned fastEmit_ISD_Constant_MVT_i32_i(MVT RetVT, uint64_t imm0) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isThumb()) && (Subtarget->useMovt())) {
    return fastEmitInst_i(ARM::t2MOVi32imm, &ARM::rGPRRegClass, imm0);
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

