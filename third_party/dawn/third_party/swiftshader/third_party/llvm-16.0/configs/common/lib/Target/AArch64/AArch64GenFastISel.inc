/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* "Fast" Instruction Selector for the AArch64 target                         *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/


// FastEmit Immediate Predicate functions.
static bool Predicate_imm0_31(int64_t Imm) {

  return ((uint64_t)Imm) < 32;

}
static bool Predicate_imm0_63(int64_t Imm) {

  return ((uint64_t)Imm) < 64;

}
static bool Predicate_imm32_0_31(int64_t Imm) {

  return ((uint64_t)Imm) < 32;

}
static bool Predicate_tbz_imm0_31_diag(int64_t Imm) {

  return (((uint32_t)Imm) < 32);

}
static bool Predicate_tbz_imm32_63(int64_t Imm) {

  return (((uint32_t)Imm) > 31) && (((uint32_t)Imm) < 64);

}
static bool Predicate_VectorIndexD(int64_t Imm) {
 return ((uint64_t)Imm) < 2; 
}
static bool Predicate_VectorIndexS(int64_t Imm) {
 return ((uint64_t)Imm) < 4; 
}
static bool Predicate_VectorIndexH(int64_t Imm) {
 return ((uint64_t)Imm) < 8; 
}
static bool Predicate_VectorIndexB(int64_t Imm) {
 return ((uint64_t)Imm) < 16; 
}
static bool Predicate_VectorIndex0(int64_t Imm) {
 return ((uint64_t)Imm) == 0; 
}
static bool Predicate_imm0_255(int64_t Imm) {

  return ((uint32_t)Imm) < 256;

}
static bool Predicate_vecshiftL64(int64_t Imm) {

  return (((uint32_t)Imm) < 64);

}
static bool Predicate_vecshiftL32(int64_t Imm) {

  return (((uint32_t)Imm) < 32);

}
static bool Predicate_vecshiftR64(int64_t Imm) {

  return (((uint32_t)Imm) > 0) && (((uint32_t)Imm) < 65);

}
static bool Predicate_vecshiftL8(int64_t Imm) {

  return (((uint32_t)Imm) < 8);

}
static bool Predicate_vecshiftL16(int64_t Imm) {

  return (((uint32_t)Imm) < 16);

}
static bool Predicate_vecshiftR8(int64_t Imm) {

  return (((uint32_t)Imm) > 0) && (((uint32_t)Imm) < 9);

}
static bool Predicate_vecshiftR16(int64_t Imm) {

  return (((uint32_t)Imm) > 0) && (((uint32_t)Imm) < 17);

}
static bool Predicate_vecshiftR32(int64_t Imm) {

  return (((uint32_t)Imm) > 0) && (((uint32_t)Imm) < 33);

}
static bool Predicate_simm8_32b(int64_t Imm) {
 return Imm >= -128 && Imm < 128; 
}
static bool Predicate_simm8_64b(int64_t Imm) {
 return Imm >= -128 && Imm < 128; 
}
static bool Predicate_uimm8_32b(int64_t Imm) {
 return Imm >= 0 && Imm < 256; 
}
static bool Predicate_uimm8_64b(int64_t Imm) {
 return Imm >= 0 && Imm < 256; 
}
static bool Predicate_simm6_32b(int64_t Imm) {
 return Imm >= -32 && Imm < 32; 
}


// FastEmit functions for AArch64ISD::THREAD_POINTER.

unsigned fastEmit_AArch64ISD_THREAD_POINTER_MVT_i64_(MVT RetVT) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_(AArch64::MOVbaseTLS, &AArch64::GPR64RegClass);
}

unsigned fastEmit_AArch64ISD_THREAD_POINTER_(MVT VT, MVT RetVT) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_AArch64ISD_THREAD_POINTER_MVT_i64_(RetVT);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_(MVT VT, MVT RetVT, unsigned Opcode) override {
  switch (Opcode) {
  case AArch64ISD::THREAD_POINTER: return fastEmit_AArch64ISD_THREAD_POINTER_(VT, RetVT);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::CALL.

unsigned fastEmit_AArch64ISD_CALL_MVT_i64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if (( MF->getSubtarget<AArch64Subtarget>().hardenSlsBlr() )) {
    return fastEmitInst_r(AArch64::BLRNoIP, &AArch64::GPR64noipRegClass, Op0);
  }
  if (( !MF->getSubtarget<AArch64Subtarget>().hardenSlsBlr() )) {
    return fastEmitInst_r(AArch64::BLR, &AArch64::GPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CALL_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_AArch64ISD_CALL_MVT_i64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::CMEQz.

unsigned fastEmit_AArch64ISD_CMEQz_MVT_v8i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMEQv8i8rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMEQz_MVT_v16i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMEQv16i8rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMEQz_MVT_v4i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMEQv4i16rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMEQz_MVT_v8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMEQv8i16rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMEQz_MVT_v2i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMEQv2i32rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMEQz_MVT_v4i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMEQv4i32rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMEQz_MVT_v1i64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMEQv1i64rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMEQz_MVT_v2i64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMEQv2i64rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMEQz_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_CMEQz_MVT_v8i8_r(RetVT, Op0);
  case MVT::v16i8: return fastEmit_AArch64ISD_CMEQz_MVT_v16i8_r(RetVT, Op0);
  case MVT::v4i16: return fastEmit_AArch64ISD_CMEQz_MVT_v4i16_r(RetVT, Op0);
  case MVT::v8i16: return fastEmit_AArch64ISD_CMEQz_MVT_v8i16_r(RetVT, Op0);
  case MVT::v2i32: return fastEmit_AArch64ISD_CMEQz_MVT_v2i32_r(RetVT, Op0);
  case MVT::v4i32: return fastEmit_AArch64ISD_CMEQz_MVT_v4i32_r(RetVT, Op0);
  case MVT::v1i64: return fastEmit_AArch64ISD_CMEQz_MVT_v1i64_r(RetVT, Op0);
  case MVT::v2i64: return fastEmit_AArch64ISD_CMEQz_MVT_v2i64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::CMGEz.

unsigned fastEmit_AArch64ISD_CMGEz_MVT_v8i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMGEv8i8rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGEz_MVT_v16i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMGEv16i8rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGEz_MVT_v4i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMGEv4i16rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGEz_MVT_v8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMGEv8i16rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGEz_MVT_v2i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMGEv2i32rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGEz_MVT_v4i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMGEv4i32rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGEz_MVT_v1i64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMGEv1i64rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGEz_MVT_v2i64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMGEv2i64rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGEz_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_CMGEz_MVT_v8i8_r(RetVT, Op0);
  case MVT::v16i8: return fastEmit_AArch64ISD_CMGEz_MVT_v16i8_r(RetVT, Op0);
  case MVT::v4i16: return fastEmit_AArch64ISD_CMGEz_MVT_v4i16_r(RetVT, Op0);
  case MVT::v8i16: return fastEmit_AArch64ISD_CMGEz_MVT_v8i16_r(RetVT, Op0);
  case MVT::v2i32: return fastEmit_AArch64ISD_CMGEz_MVT_v2i32_r(RetVT, Op0);
  case MVT::v4i32: return fastEmit_AArch64ISD_CMGEz_MVT_v4i32_r(RetVT, Op0);
  case MVT::v1i64: return fastEmit_AArch64ISD_CMGEz_MVT_v1i64_r(RetVT, Op0);
  case MVT::v2i64: return fastEmit_AArch64ISD_CMGEz_MVT_v2i64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::CMGTz.

unsigned fastEmit_AArch64ISD_CMGTz_MVT_v8i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMGTv8i8rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGTz_MVT_v16i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMGTv16i8rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGTz_MVT_v4i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMGTv4i16rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGTz_MVT_v8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMGTv8i16rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGTz_MVT_v2i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMGTv2i32rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGTz_MVT_v4i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMGTv4i32rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGTz_MVT_v1i64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMGTv1i64rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGTz_MVT_v2i64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMGTv2i64rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGTz_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_CMGTz_MVT_v8i8_r(RetVT, Op0);
  case MVT::v16i8: return fastEmit_AArch64ISD_CMGTz_MVT_v16i8_r(RetVT, Op0);
  case MVT::v4i16: return fastEmit_AArch64ISD_CMGTz_MVT_v4i16_r(RetVT, Op0);
  case MVT::v8i16: return fastEmit_AArch64ISD_CMGTz_MVT_v8i16_r(RetVT, Op0);
  case MVT::v2i32: return fastEmit_AArch64ISD_CMGTz_MVT_v2i32_r(RetVT, Op0);
  case MVT::v4i32: return fastEmit_AArch64ISD_CMGTz_MVT_v4i32_r(RetVT, Op0);
  case MVT::v1i64: return fastEmit_AArch64ISD_CMGTz_MVT_v1i64_r(RetVT, Op0);
  case MVT::v2i64: return fastEmit_AArch64ISD_CMGTz_MVT_v2i64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::CMLEz.

unsigned fastEmit_AArch64ISD_CMLEz_MVT_v8i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMLEv8i8rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMLEz_MVT_v16i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMLEv16i8rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMLEz_MVT_v4i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMLEv4i16rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMLEz_MVT_v8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMLEv8i16rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMLEz_MVT_v2i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMLEv2i32rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMLEz_MVT_v4i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMLEv4i32rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMLEz_MVT_v1i64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMLEv1i64rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMLEz_MVT_v2i64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMLEv2i64rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMLEz_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_CMLEz_MVT_v8i8_r(RetVT, Op0);
  case MVT::v16i8: return fastEmit_AArch64ISD_CMLEz_MVT_v16i8_r(RetVT, Op0);
  case MVT::v4i16: return fastEmit_AArch64ISD_CMLEz_MVT_v4i16_r(RetVT, Op0);
  case MVT::v8i16: return fastEmit_AArch64ISD_CMLEz_MVT_v8i16_r(RetVT, Op0);
  case MVT::v2i32: return fastEmit_AArch64ISD_CMLEz_MVT_v2i32_r(RetVT, Op0);
  case MVT::v4i32: return fastEmit_AArch64ISD_CMLEz_MVT_v4i32_r(RetVT, Op0);
  case MVT::v1i64: return fastEmit_AArch64ISD_CMLEz_MVT_v1i64_r(RetVT, Op0);
  case MVT::v2i64: return fastEmit_AArch64ISD_CMLEz_MVT_v2i64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::CMLTz.

unsigned fastEmit_AArch64ISD_CMLTz_MVT_v8i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMLTv8i8rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMLTz_MVT_v16i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMLTv16i8rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMLTz_MVT_v4i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMLTv4i16rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMLTz_MVT_v8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMLTv8i16rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMLTz_MVT_v2i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMLTv2i32rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMLTz_MVT_v4i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMLTv4i32rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMLTz_MVT_v1i64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMLTv1i64rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMLTz_MVT_v2i64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CMLTv2i64rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMLTz_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_CMLTz_MVT_v8i8_r(RetVT, Op0);
  case MVT::v16i8: return fastEmit_AArch64ISD_CMLTz_MVT_v16i8_r(RetVT, Op0);
  case MVT::v4i16: return fastEmit_AArch64ISD_CMLTz_MVT_v4i16_r(RetVT, Op0);
  case MVT::v8i16: return fastEmit_AArch64ISD_CMLTz_MVT_v8i16_r(RetVT, Op0);
  case MVT::v2i32: return fastEmit_AArch64ISD_CMLTz_MVT_v2i32_r(RetVT, Op0);
  case MVT::v4i32: return fastEmit_AArch64ISD_CMLTz_MVT_v4i32_r(RetVT, Op0);
  case MVT::v1i64: return fastEmit_AArch64ISD_CMLTz_MVT_v1i64_r(RetVT, Op0);
  case MVT::v2i64: return fastEmit_AArch64ISD_CMLTz_MVT_v2i64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::DUP.

unsigned fastEmit_AArch64ISD_DUP_MVT_i32_MVT_v8i8_r(unsigned Op0) {
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::DUPv8i8gpr, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_DUP_MVT_i32_MVT_v16i8_r(unsigned Op0) {
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::DUPv16i8gpr, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_DUP_MVT_i32_MVT_v4i16_r(unsigned Op0) {
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::DUPv4i16gpr, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_DUP_MVT_i32_MVT_v8i16_r(unsigned Op0) {
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::DUPv8i16gpr, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_DUP_MVT_i32_MVT_v2i32_r(unsigned Op0) {
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::DUPv2i32gpr, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_DUP_MVT_i32_MVT_v4i32_r(unsigned Op0) {
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::DUPv4i32gpr, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_DUP_MVT_i32_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_DUP_MVT_i32_MVT_v8i8_r(Op0);
  case MVT::v16i8: return fastEmit_AArch64ISD_DUP_MVT_i32_MVT_v16i8_r(Op0);
  case MVT::v4i16: return fastEmit_AArch64ISD_DUP_MVT_i32_MVT_v4i16_r(Op0);
  case MVT::v8i16: return fastEmit_AArch64ISD_DUP_MVT_i32_MVT_v8i16_r(Op0);
  case MVT::v2i32: return fastEmit_AArch64ISD_DUP_MVT_i32_MVT_v2i32_r(Op0);
  case MVT::v4i32: return fastEmit_AArch64ISD_DUP_MVT_i32_MVT_v4i32_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_AArch64ISD_DUP_MVT_i64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::DUPv2i64gpr, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_DUP_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_AArch64ISD_DUP_MVT_i32_r(RetVT, Op0);
  case MVT::i64: return fastEmit_AArch64ISD_DUP_MVT_i64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::FCMEQz.

unsigned fastEmit_AArch64ISD_FCMEQz_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCMEQv4i16rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMEQz_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCMEQv8i16rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMEQz_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCMEQv2i32rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMEQz_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCMEQv4i32rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMEQz_MVT_v1f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCMEQv1i64rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMEQz_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCMEQv2i64rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMEQz_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v4f16: return fastEmit_AArch64ISD_FCMEQz_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_AArch64ISD_FCMEQz_MVT_v8f16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_AArch64ISD_FCMEQz_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_AArch64ISD_FCMEQz_MVT_v4f32_r(RetVT, Op0);
  case MVT::v1f64: return fastEmit_AArch64ISD_FCMEQz_MVT_v1f64_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_AArch64ISD_FCMEQz_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::FCMGEz.

unsigned fastEmit_AArch64ISD_FCMGEz_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCMGEv4i16rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMGEz_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCMGEv8i16rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMGEz_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCMGEv2i32rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMGEz_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCMGEv4i32rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMGEz_MVT_v1f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCMGEv1i64rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMGEz_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCMGEv2i64rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMGEz_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v4f16: return fastEmit_AArch64ISD_FCMGEz_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_AArch64ISD_FCMGEz_MVT_v8f16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_AArch64ISD_FCMGEz_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_AArch64ISD_FCMGEz_MVT_v4f32_r(RetVT, Op0);
  case MVT::v1f64: return fastEmit_AArch64ISD_FCMGEz_MVT_v1f64_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_AArch64ISD_FCMGEz_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::FCMGTz.

unsigned fastEmit_AArch64ISD_FCMGTz_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCMGTv4i16rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMGTz_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCMGTv8i16rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMGTz_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCMGTv2i32rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMGTz_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCMGTv4i32rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMGTz_MVT_v1f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCMGTv1i64rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMGTz_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCMGTv2i64rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMGTz_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v4f16: return fastEmit_AArch64ISD_FCMGTz_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_AArch64ISD_FCMGTz_MVT_v8f16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_AArch64ISD_FCMGTz_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_AArch64ISD_FCMGTz_MVT_v4f32_r(RetVT, Op0);
  case MVT::v1f64: return fastEmit_AArch64ISD_FCMGTz_MVT_v1f64_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_AArch64ISD_FCMGTz_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::FCMLEz.

unsigned fastEmit_AArch64ISD_FCMLEz_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCMLEv4i16rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMLEz_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCMLEv8i16rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMLEz_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCMLEv2i32rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMLEz_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCMLEv4i32rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMLEz_MVT_v1f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCMLEv1i64rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMLEz_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCMLEv2i64rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMLEz_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v4f16: return fastEmit_AArch64ISD_FCMLEz_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_AArch64ISD_FCMLEz_MVT_v8f16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_AArch64ISD_FCMLEz_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_AArch64ISD_FCMLEz_MVT_v4f32_r(RetVT, Op0);
  case MVT::v1f64: return fastEmit_AArch64ISD_FCMLEz_MVT_v1f64_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_AArch64ISD_FCMLEz_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::FCMLTz.

unsigned fastEmit_AArch64ISD_FCMLTz_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCMLTv4i16rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMLTz_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCMLTv8i16rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMLTz_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCMLTv2i32rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMLTz_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCMLTv4i32rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMLTz_MVT_v1f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCMLTv1i64rz, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMLTz_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCMLTv2i64rz, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMLTz_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v4f16: return fastEmit_AArch64ISD_FCMLTz_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_AArch64ISD_FCMLTz_MVT_v8f16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_AArch64ISD_FCMLTz_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_AArch64ISD_FCMLTz_MVT_v4f32_r(RetVT, Op0);
  case MVT::v1f64: return fastEmit_AArch64ISD_FCMLTz_MVT_v1f64_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_AArch64ISD_FCMLTz_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::FRECPE.

unsigned fastEmit_AArch64ISD_FRECPE_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  return fastEmitInst_r(AArch64::FRECPEv1i32, &AArch64::FPR32RegClass, Op0);
}

unsigned fastEmit_AArch64ISD_FRECPE_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  return fastEmitInst_r(AArch64::FRECPEv1i64, &AArch64::FPR64RegClass, Op0);
}

unsigned fastEmit_AArch64ISD_FRECPE_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  return fastEmitInst_r(AArch64::FRECPEv2f32, &AArch64::FPR64RegClass, Op0);
}

unsigned fastEmit_AArch64ISD_FRECPE_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  return fastEmitInst_r(AArch64::FRECPEv4f32, &AArch64::FPR128RegClass, Op0);
}

unsigned fastEmit_AArch64ISD_FRECPE_MVT_v1f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v1f64)
    return 0;
  return fastEmitInst_r(AArch64::FRECPEv1i64, &AArch64::FPR64RegClass, Op0);
}

unsigned fastEmit_AArch64ISD_FRECPE_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  return fastEmitInst_r(AArch64::FRECPEv2f64, &AArch64::FPR128RegClass, Op0);
}

unsigned fastEmit_AArch64ISD_FRECPE_MVT_nxv8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv8f16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::FRECPE_ZZ_H, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FRECPE_MVT_nxv4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv4f32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::FRECPE_ZZ_S, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FRECPE_MVT_nxv2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv2f64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::FRECPE_ZZ_D, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FRECPE_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_AArch64ISD_FRECPE_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_AArch64ISD_FRECPE_MVT_f64_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_AArch64ISD_FRECPE_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_AArch64ISD_FRECPE_MVT_v4f32_r(RetVT, Op0);
  case MVT::v1f64: return fastEmit_AArch64ISD_FRECPE_MVT_v1f64_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_AArch64ISD_FRECPE_MVT_v2f64_r(RetVT, Op0);
  case MVT::nxv8f16: return fastEmit_AArch64ISD_FRECPE_MVT_nxv8f16_r(RetVT, Op0);
  case MVT::nxv4f32: return fastEmit_AArch64ISD_FRECPE_MVT_nxv4f32_r(RetVT, Op0);
  case MVT::nxv2f64: return fastEmit_AArch64ISD_FRECPE_MVT_nxv2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::FRSQRTE.

unsigned fastEmit_AArch64ISD_FRSQRTE_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  return fastEmitInst_r(AArch64::FRSQRTEv1i32, &AArch64::FPR32RegClass, Op0);
}

unsigned fastEmit_AArch64ISD_FRSQRTE_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  return fastEmitInst_r(AArch64::FRSQRTEv1i64, &AArch64::FPR64RegClass, Op0);
}

unsigned fastEmit_AArch64ISD_FRSQRTE_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  return fastEmitInst_r(AArch64::FRSQRTEv2f32, &AArch64::FPR64RegClass, Op0);
}

unsigned fastEmit_AArch64ISD_FRSQRTE_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  return fastEmitInst_r(AArch64::FRSQRTEv4f32, &AArch64::FPR128RegClass, Op0);
}

unsigned fastEmit_AArch64ISD_FRSQRTE_MVT_v1f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v1f64)
    return 0;
  return fastEmitInst_r(AArch64::FRSQRTEv1i64, &AArch64::FPR64RegClass, Op0);
}

unsigned fastEmit_AArch64ISD_FRSQRTE_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  return fastEmitInst_r(AArch64::FRSQRTEv2f64, &AArch64::FPR128RegClass, Op0);
}

unsigned fastEmit_AArch64ISD_FRSQRTE_MVT_nxv8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv8f16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::FRSQRTE_ZZ_H, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FRSQRTE_MVT_nxv4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv4f32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::FRSQRTE_ZZ_S, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FRSQRTE_MVT_nxv2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv2f64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::FRSQRTE_ZZ_D, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FRSQRTE_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_AArch64ISD_FRSQRTE_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_AArch64ISD_FRSQRTE_MVT_f64_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_AArch64ISD_FRSQRTE_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_AArch64ISD_FRSQRTE_MVT_v4f32_r(RetVT, Op0);
  case MVT::v1f64: return fastEmit_AArch64ISD_FRSQRTE_MVT_v1f64_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_AArch64ISD_FRSQRTE_MVT_v2f64_r(RetVT, Op0);
  case MVT::nxv8f16: return fastEmit_AArch64ISD_FRSQRTE_MVT_nxv8f16_r(RetVT, Op0);
  case MVT::nxv4f32: return fastEmit_AArch64ISD_FRSQRTE_MVT_nxv4f32_r(RetVT, Op0);
  case MVT::nxv2f64: return fastEmit_AArch64ISD_FRSQRTE_MVT_nxv2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::OBSCURE_COPY.

unsigned fastEmit_AArch64ISD_OBSCURE_COPY_MVT_i64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasSME())) {
    return fastEmitInst_r(AArch64::OBSCURE_COPY, &AArch64::GPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_OBSCURE_COPY_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_AArch64ISD_OBSCURE_COPY_MVT_i64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::REV16.

unsigned fastEmit_AArch64ISD_REV16_MVT_v8i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::REV16v8i8, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_REV16_MVT_v16i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::REV16v16i8, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_REV16_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_REV16_MVT_v8i8_r(RetVT, Op0);
  case MVT::v16i8: return fastEmit_AArch64ISD_REV16_MVT_v16i8_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::REV32.

unsigned fastEmit_AArch64ISD_REV32_MVT_v8i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::REV32v8i8, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_REV32_MVT_v16i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::REV32v16i8, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_REV32_MVT_v4i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::REV32v4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_REV32_MVT_v8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::REV32v8i16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_REV32_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  return fastEmitInst_r(AArch64::REV32v4i16, &AArch64::FPR64RegClass, Op0);
}

unsigned fastEmit_AArch64ISD_REV32_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  return fastEmitInst_r(AArch64::REV32v8i16, &AArch64::FPR128RegClass, Op0);
}

unsigned fastEmit_AArch64ISD_REV32_MVT_v4bf16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4bf16)
    return 0;
  return fastEmitInst_r(AArch64::REV32v4i16, &AArch64::FPR64RegClass, Op0);
}

unsigned fastEmit_AArch64ISD_REV32_MVT_v8bf16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8bf16)
    return 0;
  return fastEmitInst_r(AArch64::REV32v8i16, &AArch64::FPR128RegClass, Op0);
}

unsigned fastEmit_AArch64ISD_REV32_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_REV32_MVT_v8i8_r(RetVT, Op0);
  case MVT::v16i8: return fastEmit_AArch64ISD_REV32_MVT_v16i8_r(RetVT, Op0);
  case MVT::v4i16: return fastEmit_AArch64ISD_REV32_MVT_v4i16_r(RetVT, Op0);
  case MVT::v8i16: return fastEmit_AArch64ISD_REV32_MVT_v8i16_r(RetVT, Op0);
  case MVT::v4f16: return fastEmit_AArch64ISD_REV32_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_AArch64ISD_REV32_MVT_v8f16_r(RetVT, Op0);
  case MVT::v4bf16: return fastEmit_AArch64ISD_REV32_MVT_v4bf16_r(RetVT, Op0);
  case MVT::v8bf16: return fastEmit_AArch64ISD_REV32_MVT_v8bf16_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::REV64.

unsigned fastEmit_AArch64ISD_REV64_MVT_v8i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::REV64v8i8, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_REV64_MVT_v16i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::REV64v16i8, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_REV64_MVT_v4i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::REV64v4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_REV64_MVT_v8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::REV64v8i16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_REV64_MVT_v2i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::REV64v2i32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_REV64_MVT_v4i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::REV64v4i32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_REV64_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  return fastEmitInst_r(AArch64::REV64v4i16, &AArch64::FPR64RegClass, Op0);
}

unsigned fastEmit_AArch64ISD_REV64_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  return fastEmitInst_r(AArch64::REV64v8i16, &AArch64::FPR128RegClass, Op0);
}

unsigned fastEmit_AArch64ISD_REV64_MVT_v4bf16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4bf16)
    return 0;
  return fastEmitInst_r(AArch64::REV64v4i16, &AArch64::FPR64RegClass, Op0);
}

unsigned fastEmit_AArch64ISD_REV64_MVT_v8bf16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8bf16)
    return 0;
  return fastEmitInst_r(AArch64::REV64v8i16, &AArch64::FPR128RegClass, Op0);
}

unsigned fastEmit_AArch64ISD_REV64_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  return fastEmitInst_r(AArch64::REV64v2i32, &AArch64::FPR64RegClass, Op0);
}

unsigned fastEmit_AArch64ISD_REV64_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  return fastEmitInst_r(AArch64::REV64v4i32, &AArch64::FPR128RegClass, Op0);
}

unsigned fastEmit_AArch64ISD_REV64_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_REV64_MVT_v8i8_r(RetVT, Op0);
  case MVT::v16i8: return fastEmit_AArch64ISD_REV64_MVT_v16i8_r(RetVT, Op0);
  case MVT::v4i16: return fastEmit_AArch64ISD_REV64_MVT_v4i16_r(RetVT, Op0);
  case MVT::v8i16: return fastEmit_AArch64ISD_REV64_MVT_v8i16_r(RetVT, Op0);
  case MVT::v2i32: return fastEmit_AArch64ISD_REV64_MVT_v2i32_r(RetVT, Op0);
  case MVT::v4i32: return fastEmit_AArch64ISD_REV64_MVT_v4i32_r(RetVT, Op0);
  case MVT::v4f16: return fastEmit_AArch64ISD_REV64_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_AArch64ISD_REV64_MVT_v8f16_r(RetVT, Op0);
  case MVT::v4bf16: return fastEmit_AArch64ISD_REV64_MVT_v4bf16_r(RetVT, Op0);
  case MVT::v8bf16: return fastEmit_AArch64ISD_REV64_MVT_v8bf16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_AArch64ISD_REV64_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_AArch64ISD_REV64_MVT_v4f32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::SADDLP.

unsigned fastEmit_AArch64ISD_SADDLP_MVT_v8i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::SADDLPv8i8_v4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SADDLP_MVT_v16i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::SADDLPv16i8_v8i16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SADDLP_MVT_v4i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::SADDLPv4i16_v2i32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SADDLP_MVT_v8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::SADDLPv8i16_v4i32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SADDLP_MVT_v2i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::SADDLPv2i32_v1i64, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SADDLP_MVT_v4i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::SADDLPv4i32_v2i64, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SADDLP_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_SADDLP_MVT_v8i8_r(RetVT, Op0);
  case MVT::v16i8: return fastEmit_AArch64ISD_SADDLP_MVT_v16i8_r(RetVT, Op0);
  case MVT::v4i16: return fastEmit_AArch64ISD_SADDLP_MVT_v4i16_r(RetVT, Op0);
  case MVT::v8i16: return fastEmit_AArch64ISD_SADDLP_MVT_v8i16_r(RetVT, Op0);
  case MVT::v2i32: return fastEmit_AArch64ISD_SADDLP_MVT_v2i32_r(RetVT, Op0);
  case MVT::v4i32: return fastEmit_AArch64ISD_SADDLP_MVT_v4i32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::SITOF.

unsigned fastEmit_AArch64ISD_SITOF_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::SCVTFv1i16, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SITOF_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::SCVTFv1i32, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SITOF_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::SCVTFv1i64, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SITOF_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_AArch64ISD_SITOF_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_AArch64ISD_SITOF_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_AArch64ISD_SITOF_MVT_f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::SUNPKHI.

unsigned fastEmit_AArch64ISD_SUNPKHI_MVT_nxv16i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv8i16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::SUNPKHI_ZZ_H, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SUNPKHI_MVT_nxv8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv4i32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::SUNPKHI_ZZ_S, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SUNPKHI_MVT_nxv4i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv2i64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::SUNPKHI_ZZ_D, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SUNPKHI_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::nxv16i8: return fastEmit_AArch64ISD_SUNPKHI_MVT_nxv16i8_r(RetVT, Op0);
  case MVT::nxv8i16: return fastEmit_AArch64ISD_SUNPKHI_MVT_nxv8i16_r(RetVT, Op0);
  case MVT::nxv4i32: return fastEmit_AArch64ISD_SUNPKHI_MVT_nxv4i32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::SUNPKLO.

unsigned fastEmit_AArch64ISD_SUNPKLO_MVT_nxv16i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv8i16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::SUNPKLO_ZZ_H, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SUNPKLO_MVT_nxv8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv4i32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::SUNPKLO_ZZ_S, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SUNPKLO_MVT_nxv4i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv2i64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::SUNPKLO_ZZ_D, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SUNPKLO_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::nxv16i8: return fastEmit_AArch64ISD_SUNPKLO_MVT_nxv16i8_r(RetVT, Op0);
  case MVT::nxv8i16: return fastEmit_AArch64ISD_SUNPKLO_MVT_nxv8i16_r(RetVT, Op0);
  case MVT::nxv4i32: return fastEmit_AArch64ISD_SUNPKLO_MVT_nxv4i32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::UADDLP.

unsigned fastEmit_AArch64ISD_UADDLP_MVT_v8i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::UADDLPv8i8_v4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UADDLP_MVT_v16i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::UADDLPv16i8_v8i16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UADDLP_MVT_v4i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::UADDLPv4i16_v2i32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UADDLP_MVT_v8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::UADDLPv8i16_v4i32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UADDLP_MVT_v2i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::UADDLPv2i32_v1i64, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UADDLP_MVT_v4i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::UADDLPv4i32_v2i64, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UADDLP_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_UADDLP_MVT_v8i8_r(RetVT, Op0);
  case MVT::v16i8: return fastEmit_AArch64ISD_UADDLP_MVT_v16i8_r(RetVT, Op0);
  case MVT::v4i16: return fastEmit_AArch64ISD_UADDLP_MVT_v4i16_r(RetVT, Op0);
  case MVT::v8i16: return fastEmit_AArch64ISD_UADDLP_MVT_v8i16_r(RetVT, Op0);
  case MVT::v2i32: return fastEmit_AArch64ISD_UADDLP_MVT_v2i32_r(RetVT, Op0);
  case MVT::v4i32: return fastEmit_AArch64ISD_UADDLP_MVT_v4i32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::UITOF.

unsigned fastEmit_AArch64ISD_UITOF_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::UCVTFv1i16, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UITOF_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::UCVTFv1i32, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UITOF_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::UCVTFv1i64, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UITOF_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_AArch64ISD_UITOF_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_AArch64ISD_UITOF_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_AArch64ISD_UITOF_MVT_f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::UUNPKHI.

unsigned fastEmit_AArch64ISD_UUNPKHI_MVT_nxv16i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv8i16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::UUNPKHI_ZZ_H, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UUNPKHI_MVT_nxv8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv4i32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::UUNPKHI_ZZ_S, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UUNPKHI_MVT_nxv4i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv2i64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::UUNPKHI_ZZ_D, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UUNPKHI_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::nxv16i8: return fastEmit_AArch64ISD_UUNPKHI_MVT_nxv16i8_r(RetVT, Op0);
  case MVT::nxv8i16: return fastEmit_AArch64ISD_UUNPKHI_MVT_nxv8i16_r(RetVT, Op0);
  case MVT::nxv4i32: return fastEmit_AArch64ISD_UUNPKHI_MVT_nxv4i32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::UUNPKLO.

unsigned fastEmit_AArch64ISD_UUNPKLO_MVT_nxv16i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv8i16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::UUNPKLO_ZZ_H, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UUNPKLO_MVT_nxv8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv4i32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::UUNPKLO_ZZ_S, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UUNPKLO_MVT_nxv4i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv2i64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::UUNPKLO_ZZ_D, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UUNPKLO_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::nxv16i8: return fastEmit_AArch64ISD_UUNPKLO_MVT_nxv16i8_r(RetVT, Op0);
  case MVT::nxv8i16: return fastEmit_AArch64ISD_UUNPKLO_MVT_nxv8i16_r(RetVT, Op0);
  case MVT::nxv4i32: return fastEmit_AArch64ISD_UUNPKLO_MVT_nxv4i32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::ABS.

unsigned fastEmit_ISD_ABS_MVT_i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasCSSC())) {
    return fastEmitInst_r(AArch64::ABSWr, &AArch64::GPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_ABS_MVT_i64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((!Subtarget->hasCSSC())) {
    return fastEmitInst_r(AArch64::ABSv1i64, &AArch64::FPR64RegClass, Op0);
  }
  if ((Subtarget->hasCSSC())) {
    return fastEmitInst_r(AArch64::ABSXr, &AArch64::GPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_ABS_MVT_v8i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::ABSv8i8, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_ABS_MVT_v16i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::ABSv16i8, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_ABS_MVT_v4i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::ABSv4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_ABS_MVT_v8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::ABSv8i16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_ABS_MVT_v2i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::ABSv2i32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_ABS_MVT_v4i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::ABSv4i32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_ABS_MVT_v1i64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::ABSv1i64, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_ABS_MVT_v2i64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::ABSv2i64, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_ABS_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_ABS_MVT_i32_r(RetVT, Op0);
  case MVT::i64: return fastEmit_ISD_ABS_MVT_i64_r(RetVT, Op0);
  case MVT::v8i8: return fastEmit_ISD_ABS_MVT_v8i8_r(RetVT, Op0);
  case MVT::v16i8: return fastEmit_ISD_ABS_MVT_v16i8_r(RetVT, Op0);
  case MVT::v4i16: return fastEmit_ISD_ABS_MVT_v4i16_r(RetVT, Op0);
  case MVT::v8i16: return fastEmit_ISD_ABS_MVT_v8i16_r(RetVT, Op0);
  case MVT::v2i32: return fastEmit_ISD_ABS_MVT_v2i32_r(RetVT, Op0);
  case MVT::v4i32: return fastEmit_ISD_ABS_MVT_v4i32_r(RetVT, Op0);
  case MVT::v1i64: return fastEmit_ISD_ABS_MVT_v1i64_r(RetVT, Op0);
  case MVT::v2i64: return fastEmit_ISD_ABS_MVT_v2i64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::BITCAST.

unsigned fastEmit_ISD_BITCAST_MVT_f64_MVT_v8i8_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v8i8, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_f64_MVT_v4i16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_f64_MVT_v2i32_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v2i32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_f64_MVT_v4f16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_f64_MVT_v4bf16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_f64_MVT_v2f32_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v2i32, &AArch64::FPR64RegClass, Op0);
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
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v8i8, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8i8_MVT_v4i16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV16v8i8, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8i8_MVT_v2i32_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v8i8, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8i8_MVT_v1i64_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v8i8, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8i8_MVT_v4f16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV16v8i8, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8i8_MVT_v4bf16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV16v8i8, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8i8_MVT_v2f32_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v8i8, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8i8_MVT_v1f64_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v8i8, &AArch64::FPR64RegClass, Op0);
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
  case MVT::v1f64: return fastEmit_ISD_BITCAST_MVT_v8i8_MVT_v1f64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_BITCAST_MVT_v16i8_MVT_v8i16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV16v16i8, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v16i8_MVT_v4i32_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v16i8, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v16i8_MVT_v2i64_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v16i8, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v16i8_MVT_v8f16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV16v16i8, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v16i8_MVT_v8bf16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV16v16i8, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v16i8_MVT_v4f32_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v16i8, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v16i8_MVT_v2f64_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v16i8, &AArch64::FPR128RegClass, Op0);
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
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4i16_MVT_v8i8_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV16v8i8, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4i16_MVT_v2i32_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4i16_MVT_v1i64_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4i16_MVT_v2f32_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4i16_MVT_v1f64_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v4i16, &AArch64::FPR64RegClass, Op0);
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
  case MVT::v1f64: return fastEmit_ISD_BITCAST_MVT_v4i16_MVT_v1f64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_BITCAST_MVT_v8i16_MVT_v16i8_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV16v16i8, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8i16_MVT_v4i32_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v8i16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8i16_MVT_v2i64_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v8i16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8i16_MVT_v4f32_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v8i16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8i16_MVT_v2f64_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v8i16, &AArch64::FPR128RegClass, Op0);
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
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v2i32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2i32_MVT_v8i8_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v8i8, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2i32_MVT_v4i16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2i32_MVT_v1i64_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v2i32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2i32_MVT_v4f16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2i32_MVT_v4bf16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2i32_MVT_v1f64_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v2i32, &AArch64::FPR64RegClass, Op0);
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
  case MVT::v1f64: return fastEmit_ISD_BITCAST_MVT_v2i32_MVT_v1f64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_BITCAST_MVT_v4i32_MVT_v16i8_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v16i8, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4i32_MVT_v8i16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v8i16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4i32_MVT_v2i64_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v4i32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4i32_MVT_v8f16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v8i16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4i32_MVT_v8bf16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v8i16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4i32_MVT_v2f64_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v4i32, &AArch64::FPR128RegClass, Op0);
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
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v8i8, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v1i64_MVT_v4i16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v1i64_MVT_v2i32_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v2i32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v1i64_MVT_v4f16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v1i64_MVT_v4bf16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v1i64_MVT_v2f32_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v2i32, &AArch64::FPR64RegClass, Op0);
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
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v16i8, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2i64_MVT_v8i16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v8i16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2i64_MVT_v4i32_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v4i32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2i64_MVT_v8f16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v8i16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2i64_MVT_v8bf16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v8i16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2i64_MVT_v4f32_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v4i32, &AArch64::FPR128RegClass, Op0);
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
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4f16_MVT_v8i8_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV16v8i8, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4f16_MVT_v2i32_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4f16_MVT_v1i64_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4f16_MVT_v2f32_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4f16_MVT_v1f64_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v4i16, &AArch64::FPR64RegClass, Op0);
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
  case MVT::v1f64: return fastEmit_ISD_BITCAST_MVT_v4f16_MVT_v1f64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_BITCAST_MVT_v8f16_MVT_v16i8_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV16v16i8, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8f16_MVT_v4i32_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v8i16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8f16_MVT_v2i64_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v8i16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8f16_MVT_v4f32_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v8i16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8f16_MVT_v2f64_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v8i16, &AArch64::FPR128RegClass, Op0);
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
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4bf16_MVT_v8i8_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV16v8i8, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4bf16_MVT_v2i32_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4bf16_MVT_v1i64_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4bf16_MVT_v2f32_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4bf16_MVT_v1f64_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v4i16, &AArch64::FPR64RegClass, Op0);
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
  case MVT::v1f64: return fastEmit_ISD_BITCAST_MVT_v4bf16_MVT_v1f64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_BITCAST_MVT_v8bf16_MVT_v16i8_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV16v16i8, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8bf16_MVT_v4i32_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v8i16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8bf16_MVT_v2i64_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v8i16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8bf16_MVT_v4f32_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v8i16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v8bf16_MVT_v2f64_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v8i16, &AArch64::FPR128RegClass, Op0);
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
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v2i32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2f32_MVT_v8i8_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v8i8, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2f32_MVT_v4i16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2f32_MVT_v1i64_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v2i32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2f32_MVT_v4f16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2f32_MVT_v4bf16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2f32_MVT_v1f64_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v2i32, &AArch64::FPR64RegClass, Op0);
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
  case MVT::v1f64: return fastEmit_ISD_BITCAST_MVT_v2f32_MVT_v1f64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_BITCAST_MVT_v4f32_MVT_v16i8_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v16i8, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4f32_MVT_v8i16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v8i16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4f32_MVT_v2i64_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v4i32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4f32_MVT_v8f16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v8i16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4f32_MVT_v8bf16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV32v8i16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v4f32_MVT_v2f64_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v4i32, &AArch64::FPR128RegClass, Op0);
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

unsigned fastEmit_ISD_BITCAST_MVT_v1f64_MVT_v8i8_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v8i8, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v1f64_MVT_v4i16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v1f64_MVT_v2i32_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v2i32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v1f64_MVT_v4f16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v1f64_MVT_v4bf16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v1f64_MVT_v2f32_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v2i32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v1f64_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ISD_BITCAST_MVT_v1f64_MVT_v8i8_r(Op0);
  case MVT::v4i16: return fastEmit_ISD_BITCAST_MVT_v1f64_MVT_v4i16_r(Op0);
  case MVT::v2i32: return fastEmit_ISD_BITCAST_MVT_v1f64_MVT_v2i32_r(Op0);
  case MVT::v4f16: return fastEmit_ISD_BITCAST_MVT_v1f64_MVT_v4f16_r(Op0);
  case MVT::v4bf16: return fastEmit_ISD_BITCAST_MVT_v1f64_MVT_v4bf16_r(Op0);
  case MVT::v2f32: return fastEmit_ISD_BITCAST_MVT_v1f64_MVT_v2f32_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_BITCAST_MVT_v2f64_MVT_v16i8_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v16i8, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2f64_MVT_v8i16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v8i16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2f64_MVT_v4i32_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v4i32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2f64_MVT_v8f16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v8i16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2f64_MVT_v8bf16_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v8i16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_v2f64_MVT_v4f32_r(unsigned Op0) {
  if ((!Subtarget->isLittleEndian())) {
    return fastEmitInst_r(AArch64::REV64v4i32, &AArch64::FPR128RegClass, Op0);
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
  case MVT::v1f64: return fastEmit_ISD_BITCAST_MVT_v1f64_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_ISD_BITCAST_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::BITREVERSE.

unsigned fastEmit_ISD_BITREVERSE_MVT_i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_r(AArch64::RBITWr, &AArch64::GPR32RegClass, Op0);
}

unsigned fastEmit_ISD_BITREVERSE_MVT_i64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_r(AArch64::RBITXr, &AArch64::GPR64RegClass, Op0);
}

unsigned fastEmit_ISD_BITREVERSE_MVT_v8i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::RBITv8i8, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITREVERSE_MVT_v16i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::RBITv16i8, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_BITREVERSE_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_BITREVERSE_MVT_i32_r(RetVT, Op0);
  case MVT::i64: return fastEmit_ISD_BITREVERSE_MVT_i64_r(RetVT, Op0);
  case MVT::v8i8: return fastEmit_ISD_BITREVERSE_MVT_v8i8_r(RetVT, Op0);
  case MVT::v16i8: return fastEmit_ISD_BITREVERSE_MVT_v16i8_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::BRIND.

unsigned fastEmit_ISD_BRIND_MVT_i64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  return fastEmitInst_r(AArch64::BR, &AArch64::GPR64RegClass, Op0);
}

unsigned fastEmit_ISD_BRIND_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_BRIND_MVT_i64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::BSWAP.

unsigned fastEmit_ISD_BSWAP_MVT_i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_r(AArch64::REVWr, &AArch64::GPR32RegClass, Op0);
}

unsigned fastEmit_ISD_BSWAP_MVT_i64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_r(AArch64::REVXr, &AArch64::GPR64RegClass, Op0);
}

unsigned fastEmit_ISD_BSWAP_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_BSWAP_MVT_i32_r(RetVT, Op0);
  case MVT::i64: return fastEmit_ISD_BSWAP_MVT_i64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::CTLZ.

unsigned fastEmit_ISD_CTLZ_MVT_i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_r(AArch64::CLZWr, &AArch64::GPR32RegClass, Op0);
}

unsigned fastEmit_ISD_CTLZ_MVT_i64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_r(AArch64::CLZXr, &AArch64::GPR64RegClass, Op0);
}

unsigned fastEmit_ISD_CTLZ_MVT_v8i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CLZv8i8, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_CTLZ_MVT_v16i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CLZv16i8, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_CTLZ_MVT_v4i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CLZv4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_CTLZ_MVT_v8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CLZv8i16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_CTLZ_MVT_v2i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CLZv2i32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_CTLZ_MVT_v4i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CLZv4i32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_CTLZ_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_CTLZ_MVT_i32_r(RetVT, Op0);
  case MVT::i64: return fastEmit_ISD_CTLZ_MVT_i64_r(RetVT, Op0);
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

unsigned fastEmit_ISD_CTPOP_MVT_i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasCSSC())) {
    return fastEmitInst_r(AArch64::CNTWr, &AArch64::GPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_CTPOP_MVT_i64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasCSSC())) {
    return fastEmitInst_r(AArch64::CNTXr, &AArch64::GPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_CTPOP_MVT_v8i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CNTv8i8, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_CTPOP_MVT_v16i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::CNTv16i8, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_CTPOP_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_CTPOP_MVT_i32_r(RetVT, Op0);
  case MVT::i64: return fastEmit_ISD_CTPOP_MVT_i64_r(RetVT, Op0);
  case MVT::v8i8: return fastEmit_ISD_CTPOP_MVT_v8i8_r(RetVT, Op0);
  case MVT::v16i8: return fastEmit_ISD_CTPOP_MVT_v16i8_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::CTTZ.

unsigned fastEmit_ISD_CTTZ_MVT_i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasCSSC())) {
    return fastEmitInst_r(AArch64::CTZWr, &AArch64::GPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_CTTZ_MVT_i64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasCSSC())) {
    return fastEmitInst_r(AArch64::CTZXr, &AArch64::GPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_CTTZ_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_CTTZ_MVT_i32_r(RetVT, Op0);
  case MVT::i64: return fastEmit_ISD_CTTZ_MVT_i64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::FABS.

unsigned fastEmit_ISD_FABS_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FABSHr, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FABS_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FABSSr, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FABS_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FABSDr, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FABS_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FABSv4f16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FABS_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FABSv8f16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FABS_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FABSv2f32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FABS_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FABSv4f32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FABS_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FABSv2f64, &AArch64::FPR128RegClass, Op0);
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
  case MVT::v2f64: return fastEmit_ISD_FABS_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::FCEIL.

unsigned fastEmit_ISD_FCEIL_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FRINTPHr, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FCEIL_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FRINTPSr, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FCEIL_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FRINTPDr, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FCEIL_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTPv4f16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FCEIL_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTPv8f16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FCEIL_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTPv2f32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FCEIL_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTPv4f32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FCEIL_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTPv2f64, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FCEIL_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FCEIL_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_FCEIL_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_FCEIL_MVT_f64_r(RetVT, Op0);
  case MVT::v4f16: return fastEmit_ISD_FCEIL_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ISD_FCEIL_MVT_v8f16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_ISD_FCEIL_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_FCEIL_MVT_v4f32_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_ISD_FCEIL_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::FFLOOR.

unsigned fastEmit_ISD_FFLOOR_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FRINTMHr, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FFLOOR_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FRINTMSr, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FFLOOR_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FRINTMDr, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FFLOOR_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTMv4f16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FFLOOR_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTMv8f16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FFLOOR_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTMv2f32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FFLOOR_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTMv4f32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FFLOOR_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTMv2f64, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FFLOOR_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FFLOOR_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_FFLOOR_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_FFLOOR_MVT_f64_r(RetVT, Op0);
  case MVT::v4f16: return fastEmit_ISD_FFLOOR_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ISD_FFLOOR_MVT_v8f16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_ISD_FFLOOR_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_FFLOOR_MVT_v4f32_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_ISD_FFLOOR_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::FNEARBYINT.

unsigned fastEmit_ISD_FNEARBYINT_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FRINTIHr, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEARBYINT_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FRINTISr, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEARBYINT_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FRINTIDr, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEARBYINT_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTIv4f16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEARBYINT_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTIv8f16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEARBYINT_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTIv2f32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEARBYINT_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTIv4f32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEARBYINT_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTIv2f64, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEARBYINT_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FNEARBYINT_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_FNEARBYINT_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_FNEARBYINT_MVT_f64_r(RetVT, Op0);
  case MVT::v4f16: return fastEmit_ISD_FNEARBYINT_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ISD_FNEARBYINT_MVT_v8f16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_ISD_FNEARBYINT_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_FNEARBYINT_MVT_v4f32_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_ISD_FNEARBYINT_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::FNEG.

unsigned fastEmit_ISD_FNEG_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FNEGHr, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEG_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FNEGSr, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEG_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FNEGDr, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEG_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FNEGv4f16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEG_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FNEGv8f16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEG_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FNEGv2f32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEG_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FNEGv4f32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEG_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FNEGv2f64, &AArch64::FPR128RegClass, Op0);
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
  case MVT::v2f64: return fastEmit_ISD_FNEG_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::FP_EXTEND.

unsigned fastEmit_ISD_FP_EXTEND_MVT_f16_MVT_f32_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FCVTSHr, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_EXTEND_MVT_f16_MVT_f64_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FCVTDHr, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_EXTEND_MVT_f16_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FP_EXTEND_MVT_f16_MVT_f32_r(Op0);
  case MVT::f64: return fastEmit_ISD_FP_EXTEND_MVT_f16_MVT_f64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_FP_EXTEND_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FCVTDSr, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_EXTEND_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  return fastEmitInst_r(AArch64::FCVTLv4i16, &AArch64::FPR128RegClass, Op0);
}

unsigned fastEmit_ISD_FP_EXTEND_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  return fastEmitInst_r(AArch64::FCVTLv2i32, &AArch64::FPR128RegClass, Op0);
}

unsigned fastEmit_ISD_FP_EXTEND_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FP_EXTEND_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_FP_EXTEND_MVT_f32_r(RetVT, Op0);
  case MVT::v4f16: return fastEmit_ISD_FP_EXTEND_MVT_v4f16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_ISD_FP_EXTEND_MVT_v2f32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::FP_ROUND.

unsigned fastEmit_ISD_FP_ROUND_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FCVTHSr, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_ROUND_MVT_f64_MVT_f16_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FCVTHDr, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_ROUND_MVT_f64_MVT_f32_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FCVTSDr, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_ROUND_MVT_f64_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FP_ROUND_MVT_f64_MVT_f16_r(Op0);
  case MVT::f32: return fastEmit_ISD_FP_ROUND_MVT_f64_MVT_f32_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_FP_ROUND_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  return fastEmitInst_r(AArch64::FCVTNv4i16, &AArch64::FPR64RegClass, Op0);
}

unsigned fastEmit_ISD_FP_ROUND_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  return fastEmitInst_r(AArch64::FCVTNv2i32, &AArch64::FPR64RegClass, Op0);
}

unsigned fastEmit_ISD_FP_ROUND_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FP_ROUND_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_FP_ROUND_MVT_f64_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_FP_ROUND_MVT_v4f32_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_ISD_FP_ROUND_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::FP_TO_SINT.

unsigned fastEmit_ISD_FP_TO_SINT_MVT_f16_MVT_i32_r(unsigned Op0) {
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FCVTZSUWHr, &AArch64::GPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_SINT_MVT_f16_MVT_i64_r(unsigned Op0) {
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FCVTZSUXHr, &AArch64::GPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_SINT_MVT_f16_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_FP_TO_SINT_MVT_f16_MVT_i32_r(Op0);
  case MVT::i64: return fastEmit_ISD_FP_TO_SINT_MVT_f16_MVT_i64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_FP_TO_SINT_MVT_f32_MVT_i32_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FCVTZSUWSr, &AArch64::GPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_SINT_MVT_f32_MVT_i64_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FCVTZSUXSr, &AArch64::GPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_SINT_MVT_f32_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_FP_TO_SINT_MVT_f32_MVT_i32_r(Op0);
  case MVT::i64: return fastEmit_ISD_FP_TO_SINT_MVT_f32_MVT_i64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_FP_TO_SINT_MVT_f64_MVT_i32_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FCVTZSUWDr, &AArch64::GPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_SINT_MVT_f64_MVT_i64_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FCVTZSUXDr, &AArch64::GPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_SINT_MVT_f64_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_FP_TO_SINT_MVT_f64_MVT_i32_r(Op0);
  case MVT::i64: return fastEmit_ISD_FP_TO_SINT_MVT_f64_MVT_i64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_FP_TO_SINT_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCVTZSv4f16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_SINT_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCVTZSv8f16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_SINT_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCVTZSv2f32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_SINT_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCVTZSv4f32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_SINT_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCVTZSv2f64, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_SINT_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FP_TO_SINT_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_FP_TO_SINT_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_FP_TO_SINT_MVT_f64_r(RetVT, Op0);
  case MVT::v4f16: return fastEmit_ISD_FP_TO_SINT_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ISD_FP_TO_SINT_MVT_v8f16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_ISD_FP_TO_SINT_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_FP_TO_SINT_MVT_v4f32_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_ISD_FP_TO_SINT_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::FP_TO_UINT.

unsigned fastEmit_ISD_FP_TO_UINT_MVT_f16_MVT_i32_r(unsigned Op0) {
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FCVTZUUWHr, &AArch64::GPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_UINT_MVT_f16_MVT_i64_r(unsigned Op0) {
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FCVTZUUXHr, &AArch64::GPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_UINT_MVT_f16_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_FP_TO_UINT_MVT_f16_MVT_i32_r(Op0);
  case MVT::i64: return fastEmit_ISD_FP_TO_UINT_MVT_f16_MVT_i64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_FP_TO_UINT_MVT_f32_MVT_i32_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FCVTZUUWSr, &AArch64::GPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_UINT_MVT_f32_MVT_i64_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FCVTZUUXSr, &AArch64::GPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_UINT_MVT_f32_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_FP_TO_UINT_MVT_f32_MVT_i32_r(Op0);
  case MVT::i64: return fastEmit_ISD_FP_TO_UINT_MVT_f32_MVT_i64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_FP_TO_UINT_MVT_f64_MVT_i32_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FCVTZUUWDr, &AArch64::GPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_UINT_MVT_f64_MVT_i64_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FCVTZUUXDr, &AArch64::GPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_UINT_MVT_f64_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_FP_TO_UINT_MVT_f64_MVT_i32_r(Op0);
  case MVT::i64: return fastEmit_ISD_FP_TO_UINT_MVT_f64_MVT_i64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_FP_TO_UINT_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCVTZUv4f16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_UINT_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCVTZUv8f16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_UINT_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCVTZUv2f32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_UINT_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCVTZUv4f32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_UINT_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCVTZUv2f64, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_UINT_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FP_TO_UINT_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_FP_TO_UINT_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_FP_TO_UINT_MVT_f64_r(RetVT, Op0);
  case MVT::v4f16: return fastEmit_ISD_FP_TO_UINT_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ISD_FP_TO_UINT_MVT_v8f16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_ISD_FP_TO_UINT_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_FP_TO_UINT_MVT_v4f32_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_ISD_FP_TO_UINT_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::FRINT.

unsigned fastEmit_ISD_FRINT_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FRINTXHr, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FRINT_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FRINTXSr, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FRINT_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FRINTXDr, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FRINT_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTXv4f16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FRINT_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTXv8f16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FRINT_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTXv2f32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FRINT_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTXv4f32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FRINT_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTXv2f64, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FRINT_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FRINT_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_FRINT_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_FRINT_MVT_f64_r(RetVT, Op0);
  case MVT::v4f16: return fastEmit_ISD_FRINT_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ISD_FRINT_MVT_v8f16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_ISD_FRINT_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_FRINT_MVT_v4f32_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_ISD_FRINT_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::FROUND.

unsigned fastEmit_ISD_FROUND_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FRINTAHr, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FROUND_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FRINTASr, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FROUND_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FRINTADr, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FROUND_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTAv4f16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FROUND_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTAv8f16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FROUND_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTAv2f32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FROUND_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTAv4f32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FROUND_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTAv2f64, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FROUND_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FROUND_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_FROUND_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_FROUND_MVT_f64_r(RetVT, Op0);
  case MVT::v4f16: return fastEmit_ISD_FROUND_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ISD_FROUND_MVT_v8f16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_ISD_FROUND_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_FROUND_MVT_v4f32_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_ISD_FROUND_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::FROUNDEVEN.

unsigned fastEmit_ISD_FROUNDEVEN_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FRINTNHr, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FROUNDEVEN_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FRINTNSr, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FROUNDEVEN_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FRINTNDr, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FROUNDEVEN_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTNv4f16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FROUNDEVEN_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTNv8f16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FROUNDEVEN_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTNv2f32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FROUNDEVEN_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTNv4f32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FROUNDEVEN_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTNv2f64, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FROUNDEVEN_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FROUNDEVEN_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_FROUNDEVEN_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_FROUNDEVEN_MVT_f64_r(RetVT, Op0);
  case MVT::v4f16: return fastEmit_ISD_FROUNDEVEN_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ISD_FROUNDEVEN_MVT_v8f16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_ISD_FROUNDEVEN_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_FROUNDEVEN_MVT_v4f32_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_ISD_FROUNDEVEN_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::FSQRT.

unsigned fastEmit_ISD_FSQRT_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FSQRTHr, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FSQRT_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FSQRTSr, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FSQRT_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FSQRTDr, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FSQRT_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FSQRTv4f16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FSQRT_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FSQRTv8f16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FSQRT_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FSQRTv2f32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FSQRT_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FSQRTv4f32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FSQRT_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FSQRTv2f64, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FSQRT_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FSQRT_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_FSQRT_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_FSQRT_MVT_f64_r(RetVT, Op0);
  case MVT::v4f16: return fastEmit_ISD_FSQRT_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ISD_FSQRT_MVT_v8f16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_ISD_FSQRT_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_FSQRT_MVT_v4f32_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_ISD_FSQRT_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::FTRUNC.

unsigned fastEmit_ISD_FTRUNC_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FRINTZHr, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FTRUNC_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FRINTZSr, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FTRUNC_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FRINTZDr, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FTRUNC_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTZv4f16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FTRUNC_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTZv8f16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FTRUNC_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTZv2f32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FTRUNC_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTZv4f32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FTRUNC_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTZv2f64, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_FTRUNC_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FTRUNC_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_FTRUNC_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_FTRUNC_MVT_f64_r(RetVT, Op0);
  case MVT::v4f16: return fastEmit_ISD_FTRUNC_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ISD_FTRUNC_MVT_v8f16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_ISD_FTRUNC_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_FTRUNC_MVT_v4f32_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_ISD_FTRUNC_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::LLROUND.

unsigned fastEmit_ISD_LLROUND_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FCVTASUXHr, &AArch64::GPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_LLROUND_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_r(AArch64::FCVTASUXSr, &AArch64::GPR64RegClass, Op0);
}

unsigned fastEmit_ISD_LLROUND_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_r(AArch64::FCVTASUXDr, &AArch64::GPR64RegClass, Op0);
}

unsigned fastEmit_ISD_LLROUND_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_LLROUND_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_LLROUND_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_LLROUND_MVT_f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::LROUND.

unsigned fastEmit_ISD_LROUND_MVT_f16_MVT_i32_r(unsigned Op0) {
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FCVTASUWHr, &AArch64::GPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_LROUND_MVT_f16_MVT_i64_r(unsigned Op0) {
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FCVTASUXHr, &AArch64::GPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_LROUND_MVT_f16_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_LROUND_MVT_f16_MVT_i32_r(Op0);
  case MVT::i64: return fastEmit_ISD_LROUND_MVT_f16_MVT_i64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_LROUND_MVT_f32_MVT_i32_r(unsigned Op0) {
  return fastEmitInst_r(AArch64::FCVTASUWSr, &AArch64::GPR32RegClass, Op0);
}

unsigned fastEmit_ISD_LROUND_MVT_f32_MVT_i64_r(unsigned Op0) {
  return fastEmitInst_r(AArch64::FCVTASUXSr, &AArch64::GPR64RegClass, Op0);
}

unsigned fastEmit_ISD_LROUND_MVT_f32_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_LROUND_MVT_f32_MVT_i32_r(Op0);
  case MVT::i64: return fastEmit_ISD_LROUND_MVT_f32_MVT_i64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_LROUND_MVT_f64_MVT_i32_r(unsigned Op0) {
  return fastEmitInst_r(AArch64::FCVTASUWDr, &AArch64::GPR32RegClass, Op0);
}

unsigned fastEmit_ISD_LROUND_MVT_f64_MVT_i64_r(unsigned Op0) {
  return fastEmitInst_r(AArch64::FCVTASUXDr, &AArch64::GPR64RegClass, Op0);
}

unsigned fastEmit_ISD_LROUND_MVT_f64_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_LROUND_MVT_f64_MVT_i32_r(Op0);
  case MVT::i64: return fastEmit_ISD_LROUND_MVT_f64_MVT_i64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_LROUND_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_LROUND_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_LROUND_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_LROUND_MVT_f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::SINT_TO_FP.

unsigned fastEmit_ISD_SINT_TO_FP_MVT_i32_MVT_f16_r(unsigned Op0) {
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::SCVTFUWHri, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_i32_MVT_f32_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::SCVTFUWSri, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_i32_MVT_f64_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::SCVTFUWDri, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_i32_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_SINT_TO_FP_MVT_i32_MVT_f16_r(Op0);
  case MVT::f32: return fastEmit_ISD_SINT_TO_FP_MVT_i32_MVT_f32_r(Op0);
  case MVT::f64: return fastEmit_ISD_SINT_TO_FP_MVT_i32_MVT_f64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_i64_MVT_f16_r(unsigned Op0) {
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::SCVTFUXHri, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_i64_MVT_f32_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::SCVTFUXSri, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_i64_MVT_f64_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::SCVTFUXDri, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_i64_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_SINT_TO_FP_MVT_i64_MVT_f16_r(Op0);
  case MVT::f32: return fastEmit_ISD_SINT_TO_FP_MVT_i64_MVT_f32_r(Op0);
  case MVT::f64: return fastEmit_ISD_SINT_TO_FP_MVT_i64_MVT_f64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_v4i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::SCVTFv4f16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_v8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::SCVTFv8f16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_v2i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::SCVTFv2f32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_v4i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::SCVTFv4f32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_v2i64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::SCVTFv2f64, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SINT_TO_FP_MVT_i32_r(RetVT, Op0);
  case MVT::i64: return fastEmit_ISD_SINT_TO_FP_MVT_i64_r(RetVT, Op0);
  case MVT::v4i16: return fastEmit_ISD_SINT_TO_FP_MVT_v4i16_r(RetVT, Op0);
  case MVT::v8i16: return fastEmit_ISD_SINT_TO_FP_MVT_v8i16_r(RetVT, Op0);
  case MVT::v2i32: return fastEmit_ISD_SINT_TO_FP_MVT_v2i32_r(RetVT, Op0);
  case MVT::v4i32: return fastEmit_ISD_SINT_TO_FP_MVT_v4i32_r(RetVT, Op0);
  case MVT::v2i64: return fastEmit_ISD_SINT_TO_FP_MVT_v2i64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::SPLAT_VECTOR.

unsigned fastEmit_ISD_SPLAT_VECTOR_MVT_i32_MVT_nxv16i8_r(unsigned Op0) {
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::DUP_ZR_B, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_SPLAT_VECTOR_MVT_i32_MVT_nxv8i16_r(unsigned Op0) {
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::DUP_ZR_H, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_SPLAT_VECTOR_MVT_i32_MVT_nxv4i32_r(unsigned Op0) {
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::DUP_ZR_S, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_SPLAT_VECTOR_MVT_i32_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::nxv16i8: return fastEmit_ISD_SPLAT_VECTOR_MVT_i32_MVT_nxv16i8_r(Op0);
  case MVT::nxv8i16: return fastEmit_ISD_SPLAT_VECTOR_MVT_i32_MVT_nxv8i16_r(Op0);
  case MVT::nxv4i32: return fastEmit_ISD_SPLAT_VECTOR_MVT_i32_MVT_nxv4i32_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_SPLAT_VECTOR_MVT_i64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv2i64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::DUP_ZR_D, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_SPLAT_VECTOR_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SPLAT_VECTOR_MVT_i32_r(RetVT, Op0);
  case MVT::i64: return fastEmit_ISD_SPLAT_VECTOR_MVT_i64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_FCEIL.

unsigned fastEmit_ISD_STRICT_FCEIL_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FRINTPHr, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FCEIL_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FRINTPSr, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FCEIL_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FRINTPDr, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FCEIL_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTPv4f16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FCEIL_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTPv8f16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FCEIL_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTPv2f32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FCEIL_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTPv4f32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FCEIL_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTPv2f64, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FCEIL_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_STRICT_FCEIL_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_STRICT_FCEIL_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_STRICT_FCEIL_MVT_f64_r(RetVT, Op0);
  case MVT::v4f16: return fastEmit_ISD_STRICT_FCEIL_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ISD_STRICT_FCEIL_MVT_v8f16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_ISD_STRICT_FCEIL_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_STRICT_FCEIL_MVT_v4f32_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_ISD_STRICT_FCEIL_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_FFLOOR.

unsigned fastEmit_ISD_STRICT_FFLOOR_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FRINTMHr, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FFLOOR_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FRINTMSr, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FFLOOR_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FRINTMDr, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FFLOOR_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTMv4f16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FFLOOR_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTMv8f16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FFLOOR_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTMv2f32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FFLOOR_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTMv4f32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FFLOOR_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTMv2f64, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FFLOOR_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_STRICT_FFLOOR_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_STRICT_FFLOOR_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_STRICT_FFLOOR_MVT_f64_r(RetVT, Op0);
  case MVT::v4f16: return fastEmit_ISD_STRICT_FFLOOR_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ISD_STRICT_FFLOOR_MVT_v8f16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_ISD_STRICT_FFLOOR_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_STRICT_FFLOOR_MVT_v4f32_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_ISD_STRICT_FFLOOR_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_FNEARBYINT.

unsigned fastEmit_ISD_STRICT_FNEARBYINT_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FRINTIHr, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FNEARBYINT_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FRINTISr, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FNEARBYINT_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FRINTIDr, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FNEARBYINT_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTIv4f16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FNEARBYINT_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTIv8f16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FNEARBYINT_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTIv2f32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FNEARBYINT_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTIv4f32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FNEARBYINT_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTIv2f64, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FNEARBYINT_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_STRICT_FNEARBYINT_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_STRICT_FNEARBYINT_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_STRICT_FNEARBYINT_MVT_f64_r(RetVT, Op0);
  case MVT::v4f16: return fastEmit_ISD_STRICT_FNEARBYINT_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ISD_STRICT_FNEARBYINT_MVT_v8f16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_ISD_STRICT_FNEARBYINT_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_STRICT_FNEARBYINT_MVT_v4f32_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_ISD_STRICT_FNEARBYINT_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_FP_EXTEND.

unsigned fastEmit_ISD_STRICT_FP_EXTEND_MVT_f16_MVT_f32_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FCVTSHr, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_EXTEND_MVT_f16_MVT_f64_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FCVTDHr, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_EXTEND_MVT_f16_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_STRICT_FP_EXTEND_MVT_f16_MVT_f32_r(Op0);
  case MVT::f64: return fastEmit_ISD_STRICT_FP_EXTEND_MVT_f16_MVT_f64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_STRICT_FP_EXTEND_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FCVTDSr, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_EXTEND_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  return fastEmitInst_r(AArch64::FCVTLv4i16, &AArch64::FPR128RegClass, Op0);
}

unsigned fastEmit_ISD_STRICT_FP_EXTEND_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  return fastEmitInst_r(AArch64::FCVTLv2i32, &AArch64::FPR128RegClass, Op0);
}

unsigned fastEmit_ISD_STRICT_FP_EXTEND_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_STRICT_FP_EXTEND_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_STRICT_FP_EXTEND_MVT_f32_r(RetVT, Op0);
  case MVT::v4f16: return fastEmit_ISD_STRICT_FP_EXTEND_MVT_v4f16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_ISD_STRICT_FP_EXTEND_MVT_v2f32_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_FP_ROUND.

unsigned fastEmit_ISD_STRICT_FP_ROUND_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FCVTHSr, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_ROUND_MVT_f64_MVT_f16_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FCVTHDr, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_ROUND_MVT_f64_MVT_f32_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FCVTSDr, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_ROUND_MVT_f64_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_STRICT_FP_ROUND_MVT_f64_MVT_f16_r(Op0);
  case MVT::f32: return fastEmit_ISD_STRICT_FP_ROUND_MVT_f64_MVT_f32_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_STRICT_FP_ROUND_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  return fastEmitInst_r(AArch64::FCVTNv4i16, &AArch64::FPR64RegClass, Op0);
}

unsigned fastEmit_ISD_STRICT_FP_ROUND_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  return fastEmitInst_r(AArch64::FCVTNv2i32, &AArch64::FPR64RegClass, Op0);
}

unsigned fastEmit_ISD_STRICT_FP_ROUND_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_STRICT_FP_ROUND_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_STRICT_FP_ROUND_MVT_f64_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_STRICT_FP_ROUND_MVT_v4f32_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_ISD_STRICT_FP_ROUND_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_FP_TO_SINT.

unsigned fastEmit_ISD_STRICT_FP_TO_SINT_MVT_f16_MVT_i32_r(unsigned Op0) {
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FCVTZSUWHr, &AArch64::GPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_TO_SINT_MVT_f16_MVT_i64_r(unsigned Op0) {
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FCVTZSUXHr, &AArch64::GPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_TO_SINT_MVT_f16_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_STRICT_FP_TO_SINT_MVT_f16_MVT_i32_r(Op0);
  case MVT::i64: return fastEmit_ISD_STRICT_FP_TO_SINT_MVT_f16_MVT_i64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_STRICT_FP_TO_SINT_MVT_f32_MVT_i32_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FCVTZSUWSr, &AArch64::GPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_TO_SINT_MVT_f32_MVT_i64_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FCVTZSUXSr, &AArch64::GPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_TO_SINT_MVT_f32_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_STRICT_FP_TO_SINT_MVT_f32_MVT_i32_r(Op0);
  case MVT::i64: return fastEmit_ISD_STRICT_FP_TO_SINT_MVT_f32_MVT_i64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_STRICT_FP_TO_SINT_MVT_f64_MVT_i32_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FCVTZSUWDr, &AArch64::GPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_TO_SINT_MVT_f64_MVT_i64_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FCVTZSUXDr, &AArch64::GPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_TO_SINT_MVT_f64_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_STRICT_FP_TO_SINT_MVT_f64_MVT_i32_r(Op0);
  case MVT::i64: return fastEmit_ISD_STRICT_FP_TO_SINT_MVT_f64_MVT_i64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_STRICT_FP_TO_SINT_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCVTZSv4f16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_TO_SINT_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCVTZSv8f16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_TO_SINT_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCVTZSv2f32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_TO_SINT_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCVTZSv4f32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_TO_SINT_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCVTZSv2f64, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_TO_SINT_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_STRICT_FP_TO_SINT_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_STRICT_FP_TO_SINT_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_STRICT_FP_TO_SINT_MVT_f64_r(RetVT, Op0);
  case MVT::v4f16: return fastEmit_ISD_STRICT_FP_TO_SINT_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ISD_STRICT_FP_TO_SINT_MVT_v8f16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_ISD_STRICT_FP_TO_SINT_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_STRICT_FP_TO_SINT_MVT_v4f32_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_ISD_STRICT_FP_TO_SINT_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_FP_TO_UINT.

unsigned fastEmit_ISD_STRICT_FP_TO_UINT_MVT_f16_MVT_i32_r(unsigned Op0) {
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FCVTZUUWHr, &AArch64::GPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_TO_UINT_MVT_f16_MVT_i64_r(unsigned Op0) {
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FCVTZUUXHr, &AArch64::GPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_TO_UINT_MVT_f16_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_STRICT_FP_TO_UINT_MVT_f16_MVT_i32_r(Op0);
  case MVT::i64: return fastEmit_ISD_STRICT_FP_TO_UINT_MVT_f16_MVT_i64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_STRICT_FP_TO_UINT_MVT_f32_MVT_i32_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FCVTZUUWSr, &AArch64::GPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_TO_UINT_MVT_f32_MVT_i64_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FCVTZUUXSr, &AArch64::GPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_TO_UINT_MVT_f32_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_STRICT_FP_TO_UINT_MVT_f32_MVT_i32_r(Op0);
  case MVT::i64: return fastEmit_ISD_STRICT_FP_TO_UINT_MVT_f32_MVT_i64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_STRICT_FP_TO_UINT_MVT_f64_MVT_i32_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FCVTZUUWDr, &AArch64::GPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_TO_UINT_MVT_f64_MVT_i64_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FCVTZUUXDr, &AArch64::GPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_TO_UINT_MVT_f64_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_STRICT_FP_TO_UINT_MVT_f64_MVT_i32_r(Op0);
  case MVT::i64: return fastEmit_ISD_STRICT_FP_TO_UINT_MVT_f64_MVT_i64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_STRICT_FP_TO_UINT_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCVTZUv4f16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_TO_UINT_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCVTZUv8f16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_TO_UINT_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCVTZUv2f32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_TO_UINT_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCVTZUv4f32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_TO_UINT_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FCVTZUv2f64, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_TO_UINT_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_STRICT_FP_TO_UINT_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_STRICT_FP_TO_UINT_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_STRICT_FP_TO_UINT_MVT_f64_r(RetVT, Op0);
  case MVT::v4f16: return fastEmit_ISD_STRICT_FP_TO_UINT_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ISD_STRICT_FP_TO_UINT_MVT_v8f16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_ISD_STRICT_FP_TO_UINT_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_STRICT_FP_TO_UINT_MVT_v4f32_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_ISD_STRICT_FP_TO_UINT_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_FRINT.

unsigned fastEmit_ISD_STRICT_FRINT_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FRINTXHr, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FRINT_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FRINTXSr, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FRINT_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FRINTXDr, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FRINT_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTXv4f16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FRINT_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTXv8f16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FRINT_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTXv2f32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FRINT_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTXv4f32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FRINT_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTXv2f64, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FRINT_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_STRICT_FRINT_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_STRICT_FRINT_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_STRICT_FRINT_MVT_f64_r(RetVT, Op0);
  case MVT::v4f16: return fastEmit_ISD_STRICT_FRINT_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ISD_STRICT_FRINT_MVT_v8f16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_ISD_STRICT_FRINT_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_STRICT_FRINT_MVT_v4f32_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_ISD_STRICT_FRINT_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_FROUND.

unsigned fastEmit_ISD_STRICT_FROUND_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FRINTAHr, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FROUND_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FRINTASr, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FROUND_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FRINTADr, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FROUND_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTAv4f16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FROUND_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTAv8f16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FROUND_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTAv2f32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FROUND_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTAv4f32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FROUND_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTAv2f64, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FROUND_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_STRICT_FROUND_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_STRICT_FROUND_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_STRICT_FROUND_MVT_f64_r(RetVT, Op0);
  case MVT::v4f16: return fastEmit_ISD_STRICT_FROUND_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ISD_STRICT_FROUND_MVT_v8f16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_ISD_STRICT_FROUND_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_STRICT_FROUND_MVT_v4f32_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_ISD_STRICT_FROUND_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_FROUNDEVEN.

unsigned fastEmit_ISD_STRICT_FROUNDEVEN_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FRINTNHr, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FROUNDEVEN_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FRINTNSr, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FROUNDEVEN_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FRINTNDr, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FROUNDEVEN_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTNv4f16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FROUNDEVEN_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTNv8f16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FROUNDEVEN_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTNv2f32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FROUNDEVEN_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTNv4f32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FROUNDEVEN_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTNv2f64, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FROUNDEVEN_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_STRICT_FROUNDEVEN_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_STRICT_FROUNDEVEN_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_STRICT_FROUNDEVEN_MVT_f64_r(RetVT, Op0);
  case MVT::v4f16: return fastEmit_ISD_STRICT_FROUNDEVEN_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ISD_STRICT_FROUNDEVEN_MVT_v8f16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_ISD_STRICT_FROUNDEVEN_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_STRICT_FROUNDEVEN_MVT_v4f32_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_ISD_STRICT_FROUNDEVEN_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_FSQRT.

unsigned fastEmit_ISD_STRICT_FSQRT_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FSQRTHr, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSQRT_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FSQRTSr, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSQRT_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FSQRTDr, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSQRT_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FSQRTv4f16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSQRT_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FSQRTv8f16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSQRT_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FSQRTv2f32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSQRT_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FSQRTv4f32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSQRT_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FSQRTv2f64, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSQRT_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_STRICT_FSQRT_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_STRICT_FSQRT_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_STRICT_FSQRT_MVT_f64_r(RetVT, Op0);
  case MVT::v4f16: return fastEmit_ISD_STRICT_FSQRT_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ISD_STRICT_FSQRT_MVT_v8f16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_ISD_STRICT_FSQRT_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_STRICT_FSQRT_MVT_v4f32_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_ISD_STRICT_FSQRT_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_FTRUNC.

unsigned fastEmit_ISD_STRICT_FTRUNC_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FRINTZHr, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FTRUNC_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FRINTZSr, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FTRUNC_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::FRINTZDr, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FTRUNC_MVT_v4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTZv4f16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FTRUNC_MVT_v8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTZv8f16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FTRUNC_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTZv2f32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FTRUNC_MVT_v4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTZv4f32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FTRUNC_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::FRINTZv2f64, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FTRUNC_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_STRICT_FTRUNC_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_STRICT_FTRUNC_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_STRICT_FTRUNC_MVT_f64_r(RetVT, Op0);
  case MVT::v4f16: return fastEmit_ISD_STRICT_FTRUNC_MVT_v4f16_r(RetVT, Op0);
  case MVT::v8f16: return fastEmit_ISD_STRICT_FTRUNC_MVT_v8f16_r(RetVT, Op0);
  case MVT::v2f32: return fastEmit_ISD_STRICT_FTRUNC_MVT_v2f32_r(RetVT, Op0);
  case MVT::v4f32: return fastEmit_ISD_STRICT_FTRUNC_MVT_v4f32_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_ISD_STRICT_FTRUNC_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_LLROUND.

unsigned fastEmit_ISD_STRICT_LLROUND_MVT_f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FCVTASUXHr, &AArch64::GPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_LLROUND_MVT_f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_r(AArch64::FCVTASUXSr, &AArch64::GPR64RegClass, Op0);
}

unsigned fastEmit_ISD_STRICT_LLROUND_MVT_f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_r(AArch64::FCVTASUXDr, &AArch64::GPR64RegClass, Op0);
}

unsigned fastEmit_ISD_STRICT_LLROUND_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_STRICT_LLROUND_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_STRICT_LLROUND_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_STRICT_LLROUND_MVT_f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_LROUND.

unsigned fastEmit_ISD_STRICT_LROUND_MVT_f16_MVT_i32_r(unsigned Op0) {
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FCVTASUWHr, &AArch64::GPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_LROUND_MVT_f16_MVT_i64_r(unsigned Op0) {
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::FCVTASUXHr, &AArch64::GPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_LROUND_MVT_f16_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_STRICT_LROUND_MVT_f16_MVT_i32_r(Op0);
  case MVT::i64: return fastEmit_ISD_STRICT_LROUND_MVT_f16_MVT_i64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_STRICT_LROUND_MVT_f32_MVT_i32_r(unsigned Op0) {
  return fastEmitInst_r(AArch64::FCVTASUWSr, &AArch64::GPR32RegClass, Op0);
}

unsigned fastEmit_ISD_STRICT_LROUND_MVT_f32_MVT_i64_r(unsigned Op0) {
  return fastEmitInst_r(AArch64::FCVTASUXSr, &AArch64::GPR64RegClass, Op0);
}

unsigned fastEmit_ISD_STRICT_LROUND_MVT_f32_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_STRICT_LROUND_MVT_f32_MVT_i32_r(Op0);
  case MVT::i64: return fastEmit_ISD_STRICT_LROUND_MVT_f32_MVT_i64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_STRICT_LROUND_MVT_f64_MVT_i32_r(unsigned Op0) {
  return fastEmitInst_r(AArch64::FCVTASUWDr, &AArch64::GPR32RegClass, Op0);
}

unsigned fastEmit_ISD_STRICT_LROUND_MVT_f64_MVT_i64_r(unsigned Op0) {
  return fastEmitInst_r(AArch64::FCVTASUXDr, &AArch64::GPR64RegClass, Op0);
}

unsigned fastEmit_ISD_STRICT_LROUND_MVT_f64_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_STRICT_LROUND_MVT_f64_MVT_i32_r(Op0);
  case MVT::i64: return fastEmit_ISD_STRICT_LROUND_MVT_f64_MVT_i64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_STRICT_LROUND_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_STRICT_LROUND_MVT_f16_r(RetVT, Op0);
  case MVT::f32: return fastEmit_ISD_STRICT_LROUND_MVT_f32_r(RetVT, Op0);
  case MVT::f64: return fastEmit_ISD_STRICT_LROUND_MVT_f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_SINT_TO_FP.

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_i32_MVT_f16_r(unsigned Op0) {
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::SCVTFUWHri, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_i32_MVT_f32_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::SCVTFUWSri, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_i32_MVT_f64_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::SCVTFUWDri, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_i32_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_i32_MVT_f16_r(Op0);
  case MVT::f32: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_i32_MVT_f32_r(Op0);
  case MVT::f64: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_i32_MVT_f64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_i64_MVT_f16_r(unsigned Op0) {
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::SCVTFUXHri, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_i64_MVT_f32_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::SCVTFUXSri, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_i64_MVT_f64_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::SCVTFUXDri, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_i64_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_i64_MVT_f16_r(Op0);
  case MVT::f32: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_i64_MVT_f32_r(Op0);
  case MVT::f64: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_i64_MVT_f64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v4i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::SCVTFv4f16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::SCVTFv8f16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v2i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::SCVTFv2f32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v4i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::SCVTFv4f32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v2i64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::SCVTFv2f64, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_i32_r(RetVT, Op0);
  case MVT::i64: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_i64_r(RetVT, Op0);
  case MVT::v4i16: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v4i16_r(RetVT, Op0);
  case MVT::v8i16: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v8i16_r(RetVT, Op0);
  case MVT::v2i32: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v2i32_r(RetVT, Op0);
  case MVT::v4i32: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v4i32_r(RetVT, Op0);
  case MVT::v2i64: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v2i64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_UINT_TO_FP.

unsigned fastEmit_ISD_STRICT_UINT_TO_FP_MVT_i32_MVT_f16_r(unsigned Op0) {
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::UCVTFUWHri, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_UINT_TO_FP_MVT_i32_MVT_f32_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::UCVTFUWSri, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_UINT_TO_FP_MVT_i32_MVT_f64_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::UCVTFUWDri, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_UINT_TO_FP_MVT_i32_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_STRICT_UINT_TO_FP_MVT_i32_MVT_f16_r(Op0);
  case MVT::f32: return fastEmit_ISD_STRICT_UINT_TO_FP_MVT_i32_MVT_f32_r(Op0);
  case MVT::f64: return fastEmit_ISD_STRICT_UINT_TO_FP_MVT_i32_MVT_f64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_STRICT_UINT_TO_FP_MVT_i64_MVT_f16_r(unsigned Op0) {
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::UCVTFUXHri, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_UINT_TO_FP_MVT_i64_MVT_f32_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::UCVTFUXSri, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_UINT_TO_FP_MVT_i64_MVT_f64_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::UCVTFUXDri, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_UINT_TO_FP_MVT_i64_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_STRICT_UINT_TO_FP_MVT_i64_MVT_f16_r(Op0);
  case MVT::f32: return fastEmit_ISD_STRICT_UINT_TO_FP_MVT_i64_MVT_f32_r(Op0);
  case MVT::f64: return fastEmit_ISD_STRICT_UINT_TO_FP_MVT_i64_MVT_f64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v4i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::UCVTFv4f16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::UCVTFv8f16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v2i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::UCVTFv2f32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v4i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::UCVTFv4f32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v2i64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::UCVTFv2f64, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_UINT_TO_FP_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_STRICT_UINT_TO_FP_MVT_i32_r(RetVT, Op0);
  case MVT::i64: return fastEmit_ISD_STRICT_UINT_TO_FP_MVT_i64_r(RetVT, Op0);
  case MVT::v4i16: return fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v4i16_r(RetVT, Op0);
  case MVT::v8i16: return fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v8i16_r(RetVT, Op0);
  case MVT::v2i32: return fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v2i32_r(RetVT, Op0);
  case MVT::v4i32: return fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v4i32_r(RetVT, Op0);
  case MVT::v2i64: return fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v2i64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::TRUNCATE.

unsigned fastEmit_ISD_TRUNCATE_MVT_i64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_extractsubreg(RetVT, Op0, AArch64::sub_32);
}

unsigned fastEmit_ISD_TRUNCATE_MVT_v8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::XTNv8i8, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_TRUNCATE_MVT_v4i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::XTNv4i16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_TRUNCATE_MVT_v2i64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::XTNv2i32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_TRUNCATE_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_TRUNCATE_MVT_i64_r(RetVT, Op0);
  case MVT::v8i16: return fastEmit_ISD_TRUNCATE_MVT_v8i16_r(RetVT, Op0);
  case MVT::v4i32: return fastEmit_ISD_TRUNCATE_MVT_v4i32_r(RetVT, Op0);
  case MVT::v2i64: return fastEmit_ISD_TRUNCATE_MVT_v2i64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::UINT_TO_FP.

unsigned fastEmit_ISD_UINT_TO_FP_MVT_i32_MVT_f16_r(unsigned Op0) {
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::UCVTFUWHri, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_i32_MVT_f32_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::UCVTFUWSri, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_i32_MVT_f64_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::UCVTFUWDri, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_i32_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_UINT_TO_FP_MVT_i32_MVT_f16_r(Op0);
  case MVT::f32: return fastEmit_ISD_UINT_TO_FP_MVT_i32_MVT_f32_r(Op0);
  case MVT::f64: return fastEmit_ISD_UINT_TO_FP_MVT_i32_MVT_f64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_i64_MVT_f16_r(unsigned Op0) {
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_r(AArch64::UCVTFUXHri, &AArch64::FPR16RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_i64_MVT_f32_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::UCVTFUXSri, &AArch64::FPR32RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_i64_MVT_f64_r(unsigned Op0) {
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_r(AArch64::UCVTFUXDri, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_i64_r(MVT RetVT, unsigned Op0) {
switch (RetVT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_UINT_TO_FP_MVT_i64_MVT_f16_r(Op0);
  case MVT::f32: return fastEmit_ISD_UINT_TO_FP_MVT_i64_MVT_f32_r(Op0);
  case MVT::f64: return fastEmit_ISD_UINT_TO_FP_MVT_i64_MVT_f64_r(Op0);
  default: return 0;
}
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_v4i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::UCVTFv4f16, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_v8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::UCVTFv8f16, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_v2i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::UCVTFv2f32, &AArch64::FPR64RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_v4i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::UCVTFv4f32, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_v2i64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_r(AArch64::UCVTFv2f64, &AArch64::FPR128RegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_UINT_TO_FP_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_UINT_TO_FP_MVT_i32_r(RetVT, Op0);
  case MVT::i64: return fastEmit_ISD_UINT_TO_FP_MVT_i64_r(RetVT, Op0);
  case MVT::v4i16: return fastEmit_ISD_UINT_TO_FP_MVT_v4i16_r(RetVT, Op0);
  case MVT::v8i16: return fastEmit_ISD_UINT_TO_FP_MVT_v8i16_r(RetVT, Op0);
  case MVT::v2i32: return fastEmit_ISD_UINT_TO_FP_MVT_v2i32_r(RetVT, Op0);
  case MVT::v4i32: return fastEmit_ISD_UINT_TO_FP_MVT_v4i32_r(RetVT, Op0);
  case MVT::v2i64: return fastEmit_ISD_UINT_TO_FP_MVT_v2i64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::VECREDUCE_FADD.

unsigned fastEmit_ISD_VECREDUCE_FADD_MVT_v2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  return fastEmitInst_r(AArch64::FADDPv2i32p, &AArch64::FPR32RegClass, Op0);
}

unsigned fastEmit_ISD_VECREDUCE_FADD_MVT_v2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  return fastEmitInst_r(AArch64::FADDPv2i64p, &AArch64::FPR64RegClass, Op0);
}

unsigned fastEmit_ISD_VECREDUCE_FADD_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::v2f32: return fastEmit_ISD_VECREDUCE_FADD_MVT_v2f32_r(RetVT, Op0);
  case MVT::v2f64: return fastEmit_ISD_VECREDUCE_FADD_MVT_v2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for ISD::VECTOR_REVERSE.

unsigned fastEmit_ISD_VECTOR_REVERSE_MVT_nxv2i1_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv2i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::REV_PP_D, &AArch64::PPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_VECTOR_REVERSE_MVT_nxv4i1_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv4i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::REV_PP_S, &AArch64::PPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_VECTOR_REVERSE_MVT_nxv8i1_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv8i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::REV_PP_H, &AArch64::PPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_VECTOR_REVERSE_MVT_nxv16i1_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv16i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::REV_PP_B, &AArch64::PPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_VECTOR_REVERSE_MVT_nxv16i8_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv16i8)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::REV_ZZ_B, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_VECTOR_REVERSE_MVT_nxv8i16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv8i16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::REV_ZZ_H, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_VECTOR_REVERSE_MVT_nxv4i32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv4i32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::REV_ZZ_S, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_VECTOR_REVERSE_MVT_nxv2i64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv2i64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::REV_ZZ_D, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_VECTOR_REVERSE_MVT_nxv2f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv2f16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::REV_ZZ_D, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_VECTOR_REVERSE_MVT_nxv4f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv4f16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::REV_ZZ_S, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_VECTOR_REVERSE_MVT_nxv8f16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv8f16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::REV_ZZ_H, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_VECTOR_REVERSE_MVT_nxv2bf16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv2bf16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::REV_ZZ_D, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_VECTOR_REVERSE_MVT_nxv4bf16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv4bf16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::REV_ZZ_S, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_VECTOR_REVERSE_MVT_nxv8bf16_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv8bf16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::REV_ZZ_H, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_VECTOR_REVERSE_MVT_nxv2f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv2f32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::REV_ZZ_D, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_VECTOR_REVERSE_MVT_nxv4f32_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv4f32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::REV_ZZ_S, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_VECTOR_REVERSE_MVT_nxv2f64_r(MVT RetVT, unsigned Op0) {
  if (RetVT.SimpleTy != MVT::nxv2f64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_r(AArch64::REV_ZZ_D, &AArch64::ZPRRegClass, Op0);
  }
  return 0;
}

unsigned fastEmit_ISD_VECTOR_REVERSE_r(MVT VT, MVT RetVT, unsigned Op0) {
  switch (VT.SimpleTy) {
  case MVT::nxv2i1: return fastEmit_ISD_VECTOR_REVERSE_MVT_nxv2i1_r(RetVT, Op0);
  case MVT::nxv4i1: return fastEmit_ISD_VECTOR_REVERSE_MVT_nxv4i1_r(RetVT, Op0);
  case MVT::nxv8i1: return fastEmit_ISD_VECTOR_REVERSE_MVT_nxv8i1_r(RetVT, Op0);
  case MVT::nxv16i1: return fastEmit_ISD_VECTOR_REVERSE_MVT_nxv16i1_r(RetVT, Op0);
  case MVT::nxv16i8: return fastEmit_ISD_VECTOR_REVERSE_MVT_nxv16i8_r(RetVT, Op0);
  case MVT::nxv8i16: return fastEmit_ISD_VECTOR_REVERSE_MVT_nxv8i16_r(RetVT, Op0);
  case MVT::nxv4i32: return fastEmit_ISD_VECTOR_REVERSE_MVT_nxv4i32_r(RetVT, Op0);
  case MVT::nxv2i64: return fastEmit_ISD_VECTOR_REVERSE_MVT_nxv2i64_r(RetVT, Op0);
  case MVT::nxv2f16: return fastEmit_ISD_VECTOR_REVERSE_MVT_nxv2f16_r(RetVT, Op0);
  case MVT::nxv4f16: return fastEmit_ISD_VECTOR_REVERSE_MVT_nxv4f16_r(RetVT, Op0);
  case MVT::nxv8f16: return fastEmit_ISD_VECTOR_REVERSE_MVT_nxv8f16_r(RetVT, Op0);
  case MVT::nxv2bf16: return fastEmit_ISD_VECTOR_REVERSE_MVT_nxv2bf16_r(RetVT, Op0);
  case MVT::nxv4bf16: return fastEmit_ISD_VECTOR_REVERSE_MVT_nxv4bf16_r(RetVT, Op0);
  case MVT::nxv8bf16: return fastEmit_ISD_VECTOR_REVERSE_MVT_nxv8bf16_r(RetVT, Op0);
  case MVT::nxv2f32: return fastEmit_ISD_VECTOR_REVERSE_MVT_nxv2f32_r(RetVT, Op0);
  case MVT::nxv4f32: return fastEmit_ISD_VECTOR_REVERSE_MVT_nxv4f32_r(RetVT, Op0);
  case MVT::nxv2f64: return fastEmit_ISD_VECTOR_REVERSE_MVT_nxv2f64_r(RetVT, Op0);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_r(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0) override {
  switch (Opcode) {
  case AArch64ISD::CALL: return fastEmit_AArch64ISD_CALL_r(VT, RetVT, Op0);
  case AArch64ISD::CMEQz: return fastEmit_AArch64ISD_CMEQz_r(VT, RetVT, Op0);
  case AArch64ISD::CMGEz: return fastEmit_AArch64ISD_CMGEz_r(VT, RetVT, Op0);
  case AArch64ISD::CMGTz: return fastEmit_AArch64ISD_CMGTz_r(VT, RetVT, Op0);
  case AArch64ISD::CMLEz: return fastEmit_AArch64ISD_CMLEz_r(VT, RetVT, Op0);
  case AArch64ISD::CMLTz: return fastEmit_AArch64ISD_CMLTz_r(VT, RetVT, Op0);
  case AArch64ISD::DUP: return fastEmit_AArch64ISD_DUP_r(VT, RetVT, Op0);
  case AArch64ISD::FCMEQz: return fastEmit_AArch64ISD_FCMEQz_r(VT, RetVT, Op0);
  case AArch64ISD::FCMGEz: return fastEmit_AArch64ISD_FCMGEz_r(VT, RetVT, Op0);
  case AArch64ISD::FCMGTz: return fastEmit_AArch64ISD_FCMGTz_r(VT, RetVT, Op0);
  case AArch64ISD::FCMLEz: return fastEmit_AArch64ISD_FCMLEz_r(VT, RetVT, Op0);
  case AArch64ISD::FCMLTz: return fastEmit_AArch64ISD_FCMLTz_r(VT, RetVT, Op0);
  case AArch64ISD::FRECPE: return fastEmit_AArch64ISD_FRECPE_r(VT, RetVT, Op0);
  case AArch64ISD::FRSQRTE: return fastEmit_AArch64ISD_FRSQRTE_r(VT, RetVT, Op0);
  case AArch64ISD::OBSCURE_COPY: return fastEmit_AArch64ISD_OBSCURE_COPY_r(VT, RetVT, Op0);
  case AArch64ISD::REV16: return fastEmit_AArch64ISD_REV16_r(VT, RetVT, Op0);
  case AArch64ISD::REV32: return fastEmit_AArch64ISD_REV32_r(VT, RetVT, Op0);
  case AArch64ISD::REV64: return fastEmit_AArch64ISD_REV64_r(VT, RetVT, Op0);
  case AArch64ISD::SADDLP: return fastEmit_AArch64ISD_SADDLP_r(VT, RetVT, Op0);
  case AArch64ISD::SITOF: return fastEmit_AArch64ISD_SITOF_r(VT, RetVT, Op0);
  case AArch64ISD::SUNPKHI: return fastEmit_AArch64ISD_SUNPKHI_r(VT, RetVT, Op0);
  case AArch64ISD::SUNPKLO: return fastEmit_AArch64ISD_SUNPKLO_r(VT, RetVT, Op0);
  case AArch64ISD::UADDLP: return fastEmit_AArch64ISD_UADDLP_r(VT, RetVT, Op0);
  case AArch64ISD::UITOF: return fastEmit_AArch64ISD_UITOF_r(VT, RetVT, Op0);
  case AArch64ISD::UUNPKHI: return fastEmit_AArch64ISD_UUNPKHI_r(VT, RetVT, Op0);
  case AArch64ISD::UUNPKLO: return fastEmit_AArch64ISD_UUNPKLO_r(VT, RetVT, Op0);
  case ISD::ABS: return fastEmit_ISD_ABS_r(VT, RetVT, Op0);
  case ISD::BITCAST: return fastEmit_ISD_BITCAST_r(VT, RetVT, Op0);
  case ISD::BITREVERSE: return fastEmit_ISD_BITREVERSE_r(VT, RetVT, Op0);
  case ISD::BRIND: return fastEmit_ISD_BRIND_r(VT, RetVT, Op0);
  case ISD::BSWAP: return fastEmit_ISD_BSWAP_r(VT, RetVT, Op0);
  case ISD::CTLZ: return fastEmit_ISD_CTLZ_r(VT, RetVT, Op0);
  case ISD::CTPOP: return fastEmit_ISD_CTPOP_r(VT, RetVT, Op0);
  case ISD::CTTZ: return fastEmit_ISD_CTTZ_r(VT, RetVT, Op0);
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
  case ISD::FROUNDEVEN: return fastEmit_ISD_FROUNDEVEN_r(VT, RetVT, Op0);
  case ISD::FSQRT: return fastEmit_ISD_FSQRT_r(VT, RetVT, Op0);
  case ISD::FTRUNC: return fastEmit_ISD_FTRUNC_r(VT, RetVT, Op0);
  case ISD::LLROUND: return fastEmit_ISD_LLROUND_r(VT, RetVT, Op0);
  case ISD::LROUND: return fastEmit_ISD_LROUND_r(VT, RetVT, Op0);
  case ISD::SINT_TO_FP: return fastEmit_ISD_SINT_TO_FP_r(VT, RetVT, Op0);
  case ISD::SPLAT_VECTOR: return fastEmit_ISD_SPLAT_VECTOR_r(VT, RetVT, Op0);
  case ISD::STRICT_FCEIL: return fastEmit_ISD_STRICT_FCEIL_r(VT, RetVT, Op0);
  case ISD::STRICT_FFLOOR: return fastEmit_ISD_STRICT_FFLOOR_r(VT, RetVT, Op0);
  case ISD::STRICT_FNEARBYINT: return fastEmit_ISD_STRICT_FNEARBYINT_r(VT, RetVT, Op0);
  case ISD::STRICT_FP_EXTEND: return fastEmit_ISD_STRICT_FP_EXTEND_r(VT, RetVT, Op0);
  case ISD::STRICT_FP_ROUND: return fastEmit_ISD_STRICT_FP_ROUND_r(VT, RetVT, Op0);
  case ISD::STRICT_FP_TO_SINT: return fastEmit_ISD_STRICT_FP_TO_SINT_r(VT, RetVT, Op0);
  case ISD::STRICT_FP_TO_UINT: return fastEmit_ISD_STRICT_FP_TO_UINT_r(VT, RetVT, Op0);
  case ISD::STRICT_FRINT: return fastEmit_ISD_STRICT_FRINT_r(VT, RetVT, Op0);
  case ISD::STRICT_FROUND: return fastEmit_ISD_STRICT_FROUND_r(VT, RetVT, Op0);
  case ISD::STRICT_FROUNDEVEN: return fastEmit_ISD_STRICT_FROUNDEVEN_r(VT, RetVT, Op0);
  case ISD::STRICT_FSQRT: return fastEmit_ISD_STRICT_FSQRT_r(VT, RetVT, Op0);
  case ISD::STRICT_FTRUNC: return fastEmit_ISD_STRICT_FTRUNC_r(VT, RetVT, Op0);
  case ISD::STRICT_LLROUND: return fastEmit_ISD_STRICT_LLROUND_r(VT, RetVT, Op0);
  case ISD::STRICT_LROUND: return fastEmit_ISD_STRICT_LROUND_r(VT, RetVT, Op0);
  case ISD::STRICT_SINT_TO_FP: return fastEmit_ISD_STRICT_SINT_TO_FP_r(VT, RetVT, Op0);
  case ISD::STRICT_UINT_TO_FP: return fastEmit_ISD_STRICT_UINT_TO_FP_r(VT, RetVT, Op0);
  case ISD::TRUNCATE: return fastEmit_ISD_TRUNCATE_r(VT, RetVT, Op0);
  case ISD::UINT_TO_FP: return fastEmit_ISD_UINT_TO_FP_r(VT, RetVT, Op0);
  case ISD::VECREDUCE_FADD: return fastEmit_ISD_VECREDUCE_FADD_r(VT, RetVT, Op0);
  case ISD::VECTOR_REVERSE: return fastEmit_ISD_VECTOR_REVERSE_r(VT, RetVT, Op0);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::ADDP.

unsigned fastEmit_AArch64ISD_ADDP_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ADDPv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ADDP_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ADDPv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ADDP_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ADDPv4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ADDP_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ADDPv8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ADDP_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ADDPv2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ADDP_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ADDPv4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ADDP_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ADDPv2i64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ADDP_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FADDPv4f16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ADDP_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FADDPv8f16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ADDP_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FADDPv2f32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ADDP_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FADDPv4f32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ADDP_MVT_v2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FADDPv2f64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ADDP_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_ADDP_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_AArch64ISD_ADDP_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_AArch64ISD_ADDP_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_AArch64ISD_ADDP_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_AArch64ISD_ADDP_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_AArch64ISD_ADDP_MVT_v4i32_rr(RetVT, Op0, Op1);
  case MVT::v2i64: return fastEmit_AArch64ISD_ADDP_MVT_v2i64_rr(RetVT, Op0, Op1);
  case MVT::v4f16: return fastEmit_AArch64ISD_ADDP_MVT_v4f16_rr(RetVT, Op0, Op1);
  case MVT::v8f16: return fastEmit_AArch64ISD_ADDP_MVT_v8f16_rr(RetVT, Op0, Op1);
  case MVT::v2f32: return fastEmit_AArch64ISD_ADDP_MVT_v2f32_rr(RetVT, Op0, Op1);
  case MVT::v4f32: return fastEmit_AArch64ISD_ADDP_MVT_v4f32_rr(RetVT, Op0, Op1);
  case MVT::v2f64: return fastEmit_AArch64ISD_ADDP_MVT_v2f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::BIC.

unsigned fastEmit_AArch64ISD_BIC_MVT_nxv16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv16i8)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::BIC_ZZZ, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_BIC_MVT_nxv8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8i16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::BIC_ZZZ, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_BIC_MVT_nxv4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4i32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::BIC_ZZZ, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_BIC_MVT_nxv2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2i64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::BIC_ZZZ, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_BIC_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::nxv16i8: return fastEmit_AArch64ISD_BIC_MVT_nxv16i8_rr(RetVT, Op0, Op1);
  case MVT::nxv8i16: return fastEmit_AArch64ISD_BIC_MVT_nxv8i16_rr(RetVT, Op0, Op1);
  case MVT::nxv4i32: return fastEmit_AArch64ISD_BIC_MVT_nxv4i32_rr(RetVT, Op0, Op1);
  case MVT::nxv2i64: return fastEmit_AArch64ISD_BIC_MVT_nxv2i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::CMEQ.

unsigned fastEmit_AArch64ISD_CMEQ_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMEQv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMEQ_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMEQv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMEQ_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMEQv4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMEQ_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMEQv8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMEQ_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMEQv2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMEQ_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMEQv4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMEQ_MVT_v1i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMEQv1i64, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMEQ_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMEQv2i64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMEQ_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_CMEQ_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_AArch64ISD_CMEQ_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_AArch64ISD_CMEQ_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_AArch64ISD_CMEQ_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_AArch64ISD_CMEQ_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_AArch64ISD_CMEQ_MVT_v4i32_rr(RetVT, Op0, Op1);
  case MVT::v1i64: return fastEmit_AArch64ISD_CMEQ_MVT_v1i64_rr(RetVT, Op0, Op1);
  case MVT::v2i64: return fastEmit_AArch64ISD_CMEQ_MVT_v2i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::CMGE.

unsigned fastEmit_AArch64ISD_CMGE_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMGEv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGE_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMGEv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGE_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMGEv4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGE_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMGEv8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGE_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMGEv2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGE_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMGEv4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGE_MVT_v1i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMGEv1i64, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGE_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMGEv2i64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGE_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_CMGE_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_AArch64ISD_CMGE_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_AArch64ISD_CMGE_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_AArch64ISD_CMGE_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_AArch64ISD_CMGE_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_AArch64ISD_CMGE_MVT_v4i32_rr(RetVT, Op0, Op1);
  case MVT::v1i64: return fastEmit_AArch64ISD_CMGE_MVT_v1i64_rr(RetVT, Op0, Op1);
  case MVT::v2i64: return fastEmit_AArch64ISD_CMGE_MVT_v2i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::CMGT.

unsigned fastEmit_AArch64ISD_CMGT_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMGTv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGT_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMGTv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGT_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMGTv4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGT_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMGTv8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGT_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMGTv2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGT_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMGTv4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGT_MVT_v1i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMGTv1i64, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGT_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMGTv2i64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMGT_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_CMGT_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_AArch64ISD_CMGT_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_AArch64ISD_CMGT_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_AArch64ISD_CMGT_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_AArch64ISD_CMGT_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_AArch64ISD_CMGT_MVT_v4i32_rr(RetVT, Op0, Op1);
  case MVT::v1i64: return fastEmit_AArch64ISD_CMGT_MVT_v1i64_rr(RetVT, Op0, Op1);
  case MVT::v2i64: return fastEmit_AArch64ISD_CMGT_MVT_v2i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::CMHI.

unsigned fastEmit_AArch64ISD_CMHI_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMHIv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMHI_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMHIv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMHI_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMHIv4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMHI_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMHIv8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMHI_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMHIv2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMHI_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMHIv4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMHI_MVT_v1i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMHIv1i64, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMHI_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMHIv2i64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMHI_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_CMHI_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_AArch64ISD_CMHI_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_AArch64ISD_CMHI_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_AArch64ISD_CMHI_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_AArch64ISD_CMHI_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_AArch64ISD_CMHI_MVT_v4i32_rr(RetVT, Op0, Op1);
  case MVT::v1i64: return fastEmit_AArch64ISD_CMHI_MVT_v1i64_rr(RetVT, Op0, Op1);
  case MVT::v2i64: return fastEmit_AArch64ISD_CMHI_MVT_v2i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::CMHS.

unsigned fastEmit_AArch64ISD_CMHS_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMHSv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMHS_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMHSv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMHS_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMHSv4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMHS_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMHSv8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMHS_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMHSv2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMHS_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMHSv4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMHS_MVT_v1i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMHSv1i64, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMHS_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::CMHSv2i64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_CMHS_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_CMHS_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_AArch64ISD_CMHS_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_AArch64ISD_CMHS_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_AArch64ISD_CMHS_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_AArch64ISD_CMHS_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_AArch64ISD_CMHS_MVT_v4i32_rr(RetVT, Op0, Op1);
  case MVT::v1i64: return fastEmit_AArch64ISD_CMHS_MVT_v1i64_rr(RetVT, Op0, Op1);
  case MVT::v2i64: return fastEmit_AArch64ISD_CMHS_MVT_v2i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::FCMEQ.

unsigned fastEmit_AArch64ISD_FCMEQ_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FCMEQ32, &AArch64::FPR32RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMEQ_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FCMEQ64, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMEQ_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FCMEQv4f16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMEQ_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FCMEQv8f16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMEQ_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FCMEQv2f32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMEQ_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FCMEQv4f32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMEQ_MVT_v1f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FCMEQ64, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMEQ_MVT_v2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FCMEQv2f64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMEQ_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_AArch64ISD_FCMEQ_MVT_f32_rr(RetVT, Op0, Op1);
  case MVT::f64: return fastEmit_AArch64ISD_FCMEQ_MVT_f64_rr(RetVT, Op0, Op1);
  case MVT::v4f16: return fastEmit_AArch64ISD_FCMEQ_MVT_v4f16_rr(RetVT, Op0, Op1);
  case MVT::v8f16: return fastEmit_AArch64ISD_FCMEQ_MVT_v8f16_rr(RetVT, Op0, Op1);
  case MVT::v2f32: return fastEmit_AArch64ISD_FCMEQ_MVT_v2f32_rr(RetVT, Op0, Op1);
  case MVT::v4f32: return fastEmit_AArch64ISD_FCMEQ_MVT_v4f32_rr(RetVT, Op0, Op1);
  case MVT::v1f64: return fastEmit_AArch64ISD_FCMEQ_MVT_v1f64_rr(RetVT, Op0, Op1);
  case MVT::v2f64: return fastEmit_AArch64ISD_FCMEQ_MVT_v2f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::FCMGE.

unsigned fastEmit_AArch64ISD_FCMGE_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FCMGE32, &AArch64::FPR32RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMGE_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FCMGE64, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMGE_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FCMGEv4f16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMGE_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FCMGEv8f16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMGE_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FCMGEv2f32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMGE_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FCMGEv4f32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMGE_MVT_v1f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FCMGE64, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMGE_MVT_v2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FCMGEv2f64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMGE_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_AArch64ISD_FCMGE_MVT_f32_rr(RetVT, Op0, Op1);
  case MVT::f64: return fastEmit_AArch64ISD_FCMGE_MVT_f64_rr(RetVT, Op0, Op1);
  case MVT::v4f16: return fastEmit_AArch64ISD_FCMGE_MVT_v4f16_rr(RetVT, Op0, Op1);
  case MVT::v8f16: return fastEmit_AArch64ISD_FCMGE_MVT_v8f16_rr(RetVT, Op0, Op1);
  case MVT::v2f32: return fastEmit_AArch64ISD_FCMGE_MVT_v2f32_rr(RetVT, Op0, Op1);
  case MVT::v4f32: return fastEmit_AArch64ISD_FCMGE_MVT_v4f32_rr(RetVT, Op0, Op1);
  case MVT::v1f64: return fastEmit_AArch64ISD_FCMGE_MVT_v1f64_rr(RetVT, Op0, Op1);
  case MVT::v2f64: return fastEmit_AArch64ISD_FCMGE_MVT_v2f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::FCMGT.

unsigned fastEmit_AArch64ISD_FCMGT_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FCMGT32, &AArch64::FPR32RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMGT_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FCMGT64, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMGT_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FCMGTv4f16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMGT_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FCMGTv8f16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMGT_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FCMGTv2f32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMGT_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FCMGTv4f32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMGT_MVT_v1f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FCMGT64, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMGT_MVT_v2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FCMGTv2f64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMGT_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_AArch64ISD_FCMGT_MVT_f32_rr(RetVT, Op0, Op1);
  case MVT::f64: return fastEmit_AArch64ISD_FCMGT_MVT_f64_rr(RetVT, Op0, Op1);
  case MVT::v4f16: return fastEmit_AArch64ISD_FCMGT_MVT_v4f16_rr(RetVT, Op0, Op1);
  case MVT::v8f16: return fastEmit_AArch64ISD_FCMGT_MVT_v8f16_rr(RetVT, Op0, Op1);
  case MVT::v2f32: return fastEmit_AArch64ISD_FCMGT_MVT_v2f32_rr(RetVT, Op0, Op1);
  case MVT::v4f32: return fastEmit_AArch64ISD_FCMGT_MVT_v4f32_rr(RetVT, Op0, Op1);
  case MVT::v1f64: return fastEmit_AArch64ISD_FCMGT_MVT_v1f64_rr(RetVT, Op0, Op1);
  case MVT::v2f64: return fastEmit_AArch64ISD_FCMGT_MVT_v2f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::FCMP.

unsigned fastEmit_AArch64ISD_FCMP_MVT_f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_rr(AArch64::FCMPHrr, &AArch64::FPR16RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMP_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FCMPSrr, &AArch64::FPR32RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMP_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FCMPDrr, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FCMP_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_AArch64ISD_FCMP_MVT_f16_rr(RetVT, Op0, Op1);
  case MVT::f32: return fastEmit_AArch64ISD_FCMP_MVT_f32_rr(RetVT, Op0, Op1);
  case MVT::f64: return fastEmit_AArch64ISD_FCMP_MVT_f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::FRECPS.

unsigned fastEmit_AArch64ISD_FRECPS_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  return fastEmitInst_rr(AArch64::FRECPS32, &AArch64::FPR32RegClass, Op0, Op1);
}

unsigned fastEmit_AArch64ISD_FRECPS_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  return fastEmitInst_rr(AArch64::FRECPS64, &AArch64::FPR64RegClass, Op0, Op1);
}

unsigned fastEmit_AArch64ISD_FRECPS_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  return fastEmitInst_rr(AArch64::FRECPSv2f32, &AArch64::FPR64RegClass, Op0, Op1);
}

unsigned fastEmit_AArch64ISD_FRECPS_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  return fastEmitInst_rr(AArch64::FRECPSv4f32, &AArch64::FPR128RegClass, Op0, Op1);
}

unsigned fastEmit_AArch64ISD_FRECPS_MVT_v2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  return fastEmitInst_rr(AArch64::FRECPSv2f64, &AArch64::FPR128RegClass, Op0, Op1);
}

unsigned fastEmit_AArch64ISD_FRECPS_MVT_nxv8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8f16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::FRECPS_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FRECPS_MVT_nxv4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4f32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::FRECPS_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FRECPS_MVT_nxv2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2f64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::FRECPS_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FRECPS_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_AArch64ISD_FRECPS_MVT_f32_rr(RetVT, Op0, Op1);
  case MVT::f64: return fastEmit_AArch64ISD_FRECPS_MVT_f64_rr(RetVT, Op0, Op1);
  case MVT::v2f32: return fastEmit_AArch64ISD_FRECPS_MVT_v2f32_rr(RetVT, Op0, Op1);
  case MVT::v4f32: return fastEmit_AArch64ISD_FRECPS_MVT_v4f32_rr(RetVT, Op0, Op1);
  case MVT::v2f64: return fastEmit_AArch64ISD_FRECPS_MVT_v2f64_rr(RetVT, Op0, Op1);
  case MVT::nxv8f16: return fastEmit_AArch64ISD_FRECPS_MVT_nxv8f16_rr(RetVT, Op0, Op1);
  case MVT::nxv4f32: return fastEmit_AArch64ISD_FRECPS_MVT_nxv4f32_rr(RetVT, Op0, Op1);
  case MVT::nxv2f64: return fastEmit_AArch64ISD_FRECPS_MVT_nxv2f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::FRSQRTS.

unsigned fastEmit_AArch64ISD_FRSQRTS_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  return fastEmitInst_rr(AArch64::FRSQRTS32, &AArch64::FPR32RegClass, Op0, Op1);
}

unsigned fastEmit_AArch64ISD_FRSQRTS_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  return fastEmitInst_rr(AArch64::FRSQRTS64, &AArch64::FPR64RegClass, Op0, Op1);
}

unsigned fastEmit_AArch64ISD_FRSQRTS_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  return fastEmitInst_rr(AArch64::FRSQRTSv2f32, &AArch64::FPR64RegClass, Op0, Op1);
}

unsigned fastEmit_AArch64ISD_FRSQRTS_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  return fastEmitInst_rr(AArch64::FRSQRTSv4f32, &AArch64::FPR128RegClass, Op0, Op1);
}

unsigned fastEmit_AArch64ISD_FRSQRTS_MVT_v2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  return fastEmitInst_rr(AArch64::FRSQRTSv2f64, &AArch64::FPR128RegClass, Op0, Op1);
}

unsigned fastEmit_AArch64ISD_FRSQRTS_MVT_nxv8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8f16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::FRSQRTS_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FRSQRTS_MVT_nxv4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4f32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::FRSQRTS_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FRSQRTS_MVT_nxv2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2f64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::FRSQRTS_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FRSQRTS_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_AArch64ISD_FRSQRTS_MVT_f32_rr(RetVT, Op0, Op1);
  case MVT::f64: return fastEmit_AArch64ISD_FRSQRTS_MVT_f64_rr(RetVT, Op0, Op1);
  case MVT::v2f32: return fastEmit_AArch64ISD_FRSQRTS_MVT_v2f32_rr(RetVT, Op0, Op1);
  case MVT::v4f32: return fastEmit_AArch64ISD_FRSQRTS_MVT_v4f32_rr(RetVT, Op0, Op1);
  case MVT::v2f64: return fastEmit_AArch64ISD_FRSQRTS_MVT_v2f64_rr(RetVT, Op0, Op1);
  case MVT::nxv8f16: return fastEmit_AArch64ISD_FRSQRTS_MVT_nxv8f16_rr(RetVT, Op0, Op1);
  case MVT::nxv4f32: return fastEmit_AArch64ISD_FRSQRTS_MVT_nxv4f32_rr(RetVT, Op0, Op1);
  case MVT::nxv2f64: return fastEmit_AArch64ISD_FRSQRTS_MVT_nxv2f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::PMULL.

unsigned fastEmit_AArch64ISD_PMULL_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::PMULLv8i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_PMULL_MVT_v1i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasAES())) {
    return fastEmitInst_rr(AArch64::PMULLv1i64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_PMULL_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_PMULL_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v1i64: return fastEmit_AArch64ISD_PMULL_MVT_v1i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::PTEST.

unsigned fastEmit_AArch64ISD_PTEST_MVT_nxv16i1_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::PTEST_PP, &AArch64::PPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_PTEST_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::nxv16i1: return fastEmit_AArch64ISD_PTEST_MVT_nxv16i1_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::PTEST_ANY.

unsigned fastEmit_AArch64ISD_PTEST_ANY_MVT_nxv16i1_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::PTEST_PP_ANY, &AArch64::PPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_PTEST_ANY_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::nxv16i1: return fastEmit_AArch64ISD_PTEST_ANY_MVT_nxv16i1_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::SMULL.

unsigned fastEmit_AArch64ISD_SMULL_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SMULLv8i8_v8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SMULL_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SMULLv4i16_v4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SMULL_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SMULLv2i32_v2i64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SMULL_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_SMULL_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_AArch64ISD_SMULL_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_AArch64ISD_SMULL_MVT_v2i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::STRICT_FCMP.

unsigned fastEmit_AArch64ISD_STRICT_FCMP_MVT_f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_rr(AArch64::FCMPHrr, &AArch64::FPR16RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_STRICT_FCMP_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FCMPSrr, &AArch64::FPR32RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_STRICT_FCMP_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FCMPDrr, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_STRICT_FCMP_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_AArch64ISD_STRICT_FCMP_MVT_f16_rr(RetVT, Op0, Op1);
  case MVT::f32: return fastEmit_AArch64ISD_STRICT_FCMP_MVT_f32_rr(RetVT, Op0, Op1);
  case MVT::f64: return fastEmit_AArch64ISD_STRICT_FCMP_MVT_f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::STRICT_FCMPE.

unsigned fastEmit_AArch64ISD_STRICT_FCMPE_MVT_f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_rr(AArch64::FCMPEHrr, &AArch64::FPR16RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_STRICT_FCMPE_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FCMPESrr, &AArch64::FPR32RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_STRICT_FCMPE_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FCMPEDrr, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_STRICT_FCMPE_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_AArch64ISD_STRICT_FCMPE_MVT_f16_rr(RetVT, Op0, Op1);
  case MVT::f32: return fastEmit_AArch64ISD_STRICT_FCMPE_MVT_f32_rr(RetVT, Op0, Op1);
  case MVT::f64: return fastEmit_AArch64ISD_STRICT_FCMPE_MVT_f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::TBL.

unsigned fastEmit_AArch64ISD_TBL_MVT_nxv16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv16i8)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TBL_ZZZ_B, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TBL_MVT_nxv8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8i16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TBL_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TBL_MVT_nxv4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4i32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TBL_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TBL_MVT_nxv2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2i64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TBL_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TBL_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::nxv16i8: return fastEmit_AArch64ISD_TBL_MVT_nxv16i8_rr(RetVT, Op0, Op1);
  case MVT::nxv8i16: return fastEmit_AArch64ISD_TBL_MVT_nxv8i16_rr(RetVT, Op0, Op1);
  case MVT::nxv4i32: return fastEmit_AArch64ISD_TBL_MVT_nxv4i32_rr(RetVT, Op0, Op1);
  case MVT::nxv2i64: return fastEmit_AArch64ISD_TBL_MVT_nxv2i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::TRN1.

unsigned fastEmit_AArch64ISD_TRN1_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::TRN1v8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN1_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::TRN1v16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN1_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::TRN1v4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN1_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::TRN1v8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN1_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::TRN1v2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN1_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::TRN1v4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN1_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::TRN1v2i64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN1_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::TRN1v4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN1_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::TRN1v8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN1_MVT_v4bf16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4bf16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::TRN1v4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN1_MVT_v8bf16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8bf16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::TRN1v8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN1_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::TRN1v2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN1_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::TRN1v4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN1_MVT_v2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::TRN1v2i64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN1_MVT_nxv2i1_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TRN1_PPP_D, &AArch64::PPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN1_MVT_nxv4i1_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TRN1_PPP_S, &AArch64::PPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN1_MVT_nxv8i1_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TRN1_PPP_H, &AArch64::PPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN1_MVT_nxv16i1_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv16i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TRN1_PPP_B, &AArch64::PPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN1_MVT_nxv16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv16i8)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TRN1_ZZZ_B, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN1_MVT_nxv8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8i16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TRN1_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN1_MVT_nxv4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4i32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TRN1_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN1_MVT_nxv2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2i64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TRN1_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN1_MVT_nxv2f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2f16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TRN1_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN1_MVT_nxv4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4f16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TRN1_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN1_MVT_nxv8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8f16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TRN1_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN1_MVT_nxv8bf16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8bf16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TRN1_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN1_MVT_nxv2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2f32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TRN1_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN1_MVT_nxv4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4f32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TRN1_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN1_MVT_nxv2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2f64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TRN1_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN1_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_TRN1_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_AArch64ISD_TRN1_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_AArch64ISD_TRN1_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_AArch64ISD_TRN1_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_AArch64ISD_TRN1_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_AArch64ISD_TRN1_MVT_v4i32_rr(RetVT, Op0, Op1);
  case MVT::v2i64: return fastEmit_AArch64ISD_TRN1_MVT_v2i64_rr(RetVT, Op0, Op1);
  case MVT::v4f16: return fastEmit_AArch64ISD_TRN1_MVT_v4f16_rr(RetVT, Op0, Op1);
  case MVT::v8f16: return fastEmit_AArch64ISD_TRN1_MVT_v8f16_rr(RetVT, Op0, Op1);
  case MVT::v4bf16: return fastEmit_AArch64ISD_TRN1_MVT_v4bf16_rr(RetVT, Op0, Op1);
  case MVT::v8bf16: return fastEmit_AArch64ISD_TRN1_MVT_v8bf16_rr(RetVT, Op0, Op1);
  case MVT::v2f32: return fastEmit_AArch64ISD_TRN1_MVT_v2f32_rr(RetVT, Op0, Op1);
  case MVT::v4f32: return fastEmit_AArch64ISD_TRN1_MVT_v4f32_rr(RetVT, Op0, Op1);
  case MVT::v2f64: return fastEmit_AArch64ISD_TRN1_MVT_v2f64_rr(RetVT, Op0, Op1);
  case MVT::nxv2i1: return fastEmit_AArch64ISD_TRN1_MVT_nxv2i1_rr(RetVT, Op0, Op1);
  case MVT::nxv4i1: return fastEmit_AArch64ISD_TRN1_MVT_nxv4i1_rr(RetVT, Op0, Op1);
  case MVT::nxv8i1: return fastEmit_AArch64ISD_TRN1_MVT_nxv8i1_rr(RetVT, Op0, Op1);
  case MVT::nxv16i1: return fastEmit_AArch64ISD_TRN1_MVT_nxv16i1_rr(RetVT, Op0, Op1);
  case MVT::nxv16i8: return fastEmit_AArch64ISD_TRN1_MVT_nxv16i8_rr(RetVT, Op0, Op1);
  case MVT::nxv8i16: return fastEmit_AArch64ISD_TRN1_MVT_nxv8i16_rr(RetVT, Op0, Op1);
  case MVT::nxv4i32: return fastEmit_AArch64ISD_TRN1_MVT_nxv4i32_rr(RetVT, Op0, Op1);
  case MVT::nxv2i64: return fastEmit_AArch64ISD_TRN1_MVT_nxv2i64_rr(RetVT, Op0, Op1);
  case MVT::nxv2f16: return fastEmit_AArch64ISD_TRN1_MVT_nxv2f16_rr(RetVT, Op0, Op1);
  case MVT::nxv4f16: return fastEmit_AArch64ISD_TRN1_MVT_nxv4f16_rr(RetVT, Op0, Op1);
  case MVT::nxv8f16: return fastEmit_AArch64ISD_TRN1_MVT_nxv8f16_rr(RetVT, Op0, Op1);
  case MVT::nxv8bf16: return fastEmit_AArch64ISD_TRN1_MVT_nxv8bf16_rr(RetVT, Op0, Op1);
  case MVT::nxv2f32: return fastEmit_AArch64ISD_TRN1_MVT_nxv2f32_rr(RetVT, Op0, Op1);
  case MVT::nxv4f32: return fastEmit_AArch64ISD_TRN1_MVT_nxv4f32_rr(RetVT, Op0, Op1);
  case MVT::nxv2f64: return fastEmit_AArch64ISD_TRN1_MVT_nxv2f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::TRN2.

unsigned fastEmit_AArch64ISD_TRN2_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::TRN2v8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN2_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::TRN2v16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN2_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::TRN2v4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN2_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::TRN2v8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN2_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::TRN2v2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN2_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::TRN2v4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN2_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::TRN2v2i64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN2_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::TRN2v4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN2_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::TRN2v8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN2_MVT_v4bf16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4bf16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::TRN2v4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN2_MVT_v8bf16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8bf16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::TRN2v8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN2_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::TRN2v2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN2_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::TRN2v4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN2_MVT_v2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::TRN2v2i64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN2_MVT_nxv2i1_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TRN2_PPP_D, &AArch64::PPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN2_MVT_nxv4i1_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TRN2_PPP_S, &AArch64::PPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN2_MVT_nxv8i1_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TRN2_PPP_H, &AArch64::PPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN2_MVT_nxv16i1_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv16i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TRN2_PPP_B, &AArch64::PPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN2_MVT_nxv16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv16i8)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TRN2_ZZZ_B, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN2_MVT_nxv8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8i16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TRN2_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN2_MVT_nxv4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4i32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TRN2_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN2_MVT_nxv2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2i64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TRN2_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN2_MVT_nxv2f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2f16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TRN2_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN2_MVT_nxv4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4f16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TRN2_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN2_MVT_nxv8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8f16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TRN2_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN2_MVT_nxv8bf16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8bf16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TRN2_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN2_MVT_nxv2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2f32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TRN2_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN2_MVT_nxv4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4f32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TRN2_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN2_MVT_nxv2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2f64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::TRN2_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_TRN2_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_TRN2_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_AArch64ISD_TRN2_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_AArch64ISD_TRN2_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_AArch64ISD_TRN2_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_AArch64ISD_TRN2_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_AArch64ISD_TRN2_MVT_v4i32_rr(RetVT, Op0, Op1);
  case MVT::v2i64: return fastEmit_AArch64ISD_TRN2_MVT_v2i64_rr(RetVT, Op0, Op1);
  case MVT::v4f16: return fastEmit_AArch64ISD_TRN2_MVT_v4f16_rr(RetVT, Op0, Op1);
  case MVT::v8f16: return fastEmit_AArch64ISD_TRN2_MVT_v8f16_rr(RetVT, Op0, Op1);
  case MVT::v4bf16: return fastEmit_AArch64ISD_TRN2_MVT_v4bf16_rr(RetVT, Op0, Op1);
  case MVT::v8bf16: return fastEmit_AArch64ISD_TRN2_MVT_v8bf16_rr(RetVT, Op0, Op1);
  case MVT::v2f32: return fastEmit_AArch64ISD_TRN2_MVT_v2f32_rr(RetVT, Op0, Op1);
  case MVT::v4f32: return fastEmit_AArch64ISD_TRN2_MVT_v4f32_rr(RetVT, Op0, Op1);
  case MVT::v2f64: return fastEmit_AArch64ISD_TRN2_MVT_v2f64_rr(RetVT, Op0, Op1);
  case MVT::nxv2i1: return fastEmit_AArch64ISD_TRN2_MVT_nxv2i1_rr(RetVT, Op0, Op1);
  case MVT::nxv4i1: return fastEmit_AArch64ISD_TRN2_MVT_nxv4i1_rr(RetVT, Op0, Op1);
  case MVT::nxv8i1: return fastEmit_AArch64ISD_TRN2_MVT_nxv8i1_rr(RetVT, Op0, Op1);
  case MVT::nxv16i1: return fastEmit_AArch64ISD_TRN2_MVT_nxv16i1_rr(RetVT, Op0, Op1);
  case MVT::nxv16i8: return fastEmit_AArch64ISD_TRN2_MVT_nxv16i8_rr(RetVT, Op0, Op1);
  case MVT::nxv8i16: return fastEmit_AArch64ISD_TRN2_MVT_nxv8i16_rr(RetVT, Op0, Op1);
  case MVT::nxv4i32: return fastEmit_AArch64ISD_TRN2_MVT_nxv4i32_rr(RetVT, Op0, Op1);
  case MVT::nxv2i64: return fastEmit_AArch64ISD_TRN2_MVT_nxv2i64_rr(RetVT, Op0, Op1);
  case MVT::nxv2f16: return fastEmit_AArch64ISD_TRN2_MVT_nxv2f16_rr(RetVT, Op0, Op1);
  case MVT::nxv4f16: return fastEmit_AArch64ISD_TRN2_MVT_nxv4f16_rr(RetVT, Op0, Op1);
  case MVT::nxv8f16: return fastEmit_AArch64ISD_TRN2_MVT_nxv8f16_rr(RetVT, Op0, Op1);
  case MVT::nxv8bf16: return fastEmit_AArch64ISD_TRN2_MVT_nxv8bf16_rr(RetVT, Op0, Op1);
  case MVT::nxv2f32: return fastEmit_AArch64ISD_TRN2_MVT_nxv2f32_rr(RetVT, Op0, Op1);
  case MVT::nxv4f32: return fastEmit_AArch64ISD_TRN2_MVT_nxv4f32_rr(RetVT, Op0, Op1);
  case MVT::nxv2f64: return fastEmit_AArch64ISD_TRN2_MVT_nxv2f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::UMULL.

unsigned fastEmit_AArch64ISD_UMULL_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UMULLv8i8_v8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UMULL_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UMULLv4i16_v4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UMULL_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UMULLv2i32_v2i64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UMULL_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_UMULL_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_AArch64ISD_UMULL_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_AArch64ISD_UMULL_MVT_v2i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::UZP1.

unsigned fastEmit_AArch64ISD_UZP1_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UZP1v8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP1_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UZP1v16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP1_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UZP1v4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP1_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UZP1v8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP1_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UZP1v2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP1_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UZP1v4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP1_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UZP1v2i64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP1_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UZP1v4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP1_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UZP1v8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP1_MVT_v4bf16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4bf16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UZP1v4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP1_MVT_v8bf16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8bf16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UZP1v8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP1_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UZP1v2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP1_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UZP1v4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP1_MVT_v2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UZP1v2i64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP1_MVT_nxv2i1_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP1_PPP_D, &AArch64::PPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP1_MVT_nxv4i1_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP1_PPP_S, &AArch64::PPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP1_MVT_nxv8i1_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP1_PPP_H, &AArch64::PPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP1_MVT_nxv16i1_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv16i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP1_PPP_B, &AArch64::PPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP1_MVT_nxv16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv16i8)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP1_ZZZ_B, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP1_MVT_nxv8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8i16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP1_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP1_MVT_nxv4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4i32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP1_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP1_MVT_nxv2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2i64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP1_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP1_MVT_nxv2f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2f16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP1_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP1_MVT_nxv4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4f16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP1_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP1_MVT_nxv8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8f16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP1_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP1_MVT_nxv8bf16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8bf16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP1_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP1_MVT_nxv2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2f32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP1_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP1_MVT_nxv4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4f32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP1_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP1_MVT_nxv2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2f64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP1_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP1_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_UZP1_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_AArch64ISD_UZP1_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_AArch64ISD_UZP1_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_AArch64ISD_UZP1_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_AArch64ISD_UZP1_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_AArch64ISD_UZP1_MVT_v4i32_rr(RetVT, Op0, Op1);
  case MVT::v2i64: return fastEmit_AArch64ISD_UZP1_MVT_v2i64_rr(RetVT, Op0, Op1);
  case MVT::v4f16: return fastEmit_AArch64ISD_UZP1_MVT_v4f16_rr(RetVT, Op0, Op1);
  case MVT::v8f16: return fastEmit_AArch64ISD_UZP1_MVT_v8f16_rr(RetVT, Op0, Op1);
  case MVT::v4bf16: return fastEmit_AArch64ISD_UZP1_MVT_v4bf16_rr(RetVT, Op0, Op1);
  case MVT::v8bf16: return fastEmit_AArch64ISD_UZP1_MVT_v8bf16_rr(RetVT, Op0, Op1);
  case MVT::v2f32: return fastEmit_AArch64ISD_UZP1_MVT_v2f32_rr(RetVT, Op0, Op1);
  case MVT::v4f32: return fastEmit_AArch64ISD_UZP1_MVT_v4f32_rr(RetVT, Op0, Op1);
  case MVT::v2f64: return fastEmit_AArch64ISD_UZP1_MVT_v2f64_rr(RetVT, Op0, Op1);
  case MVT::nxv2i1: return fastEmit_AArch64ISD_UZP1_MVT_nxv2i1_rr(RetVT, Op0, Op1);
  case MVT::nxv4i1: return fastEmit_AArch64ISD_UZP1_MVT_nxv4i1_rr(RetVT, Op0, Op1);
  case MVT::nxv8i1: return fastEmit_AArch64ISD_UZP1_MVT_nxv8i1_rr(RetVT, Op0, Op1);
  case MVT::nxv16i1: return fastEmit_AArch64ISD_UZP1_MVT_nxv16i1_rr(RetVT, Op0, Op1);
  case MVT::nxv16i8: return fastEmit_AArch64ISD_UZP1_MVT_nxv16i8_rr(RetVT, Op0, Op1);
  case MVT::nxv8i16: return fastEmit_AArch64ISD_UZP1_MVT_nxv8i16_rr(RetVT, Op0, Op1);
  case MVT::nxv4i32: return fastEmit_AArch64ISD_UZP1_MVT_nxv4i32_rr(RetVT, Op0, Op1);
  case MVT::nxv2i64: return fastEmit_AArch64ISD_UZP1_MVT_nxv2i64_rr(RetVT, Op0, Op1);
  case MVT::nxv2f16: return fastEmit_AArch64ISD_UZP1_MVT_nxv2f16_rr(RetVT, Op0, Op1);
  case MVT::nxv4f16: return fastEmit_AArch64ISD_UZP1_MVT_nxv4f16_rr(RetVT, Op0, Op1);
  case MVT::nxv8f16: return fastEmit_AArch64ISD_UZP1_MVT_nxv8f16_rr(RetVT, Op0, Op1);
  case MVT::nxv8bf16: return fastEmit_AArch64ISD_UZP1_MVT_nxv8bf16_rr(RetVT, Op0, Op1);
  case MVT::nxv2f32: return fastEmit_AArch64ISD_UZP1_MVT_nxv2f32_rr(RetVT, Op0, Op1);
  case MVT::nxv4f32: return fastEmit_AArch64ISD_UZP1_MVT_nxv4f32_rr(RetVT, Op0, Op1);
  case MVT::nxv2f64: return fastEmit_AArch64ISD_UZP1_MVT_nxv2f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::UZP2.

unsigned fastEmit_AArch64ISD_UZP2_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UZP2v8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP2_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UZP2v16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP2_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UZP2v4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP2_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UZP2v8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP2_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UZP2v2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP2_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UZP2v4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP2_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UZP2v2i64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP2_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UZP2v4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP2_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UZP2v8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP2_MVT_v4bf16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4bf16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UZP2v4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP2_MVT_v8bf16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8bf16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UZP2v8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP2_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UZP2v2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP2_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UZP2v4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP2_MVT_v2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UZP2v2i64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP2_MVT_nxv2i1_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP2_PPP_D, &AArch64::PPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP2_MVT_nxv4i1_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP2_PPP_S, &AArch64::PPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP2_MVT_nxv8i1_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP2_PPP_H, &AArch64::PPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP2_MVT_nxv16i1_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv16i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP2_PPP_B, &AArch64::PPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP2_MVT_nxv16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv16i8)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP2_ZZZ_B, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP2_MVT_nxv8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8i16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP2_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP2_MVT_nxv4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4i32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP2_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP2_MVT_nxv2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2i64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP2_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP2_MVT_nxv2f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2f16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP2_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP2_MVT_nxv4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4f16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP2_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP2_MVT_nxv8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8f16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP2_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP2_MVT_nxv8bf16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8bf16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP2_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP2_MVT_nxv2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2f32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP2_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP2_MVT_nxv4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4f32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP2_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP2_MVT_nxv2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2f64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP2_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UZP2_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_UZP2_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_AArch64ISD_UZP2_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_AArch64ISD_UZP2_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_AArch64ISD_UZP2_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_AArch64ISD_UZP2_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_AArch64ISD_UZP2_MVT_v4i32_rr(RetVT, Op0, Op1);
  case MVT::v2i64: return fastEmit_AArch64ISD_UZP2_MVT_v2i64_rr(RetVT, Op0, Op1);
  case MVT::v4f16: return fastEmit_AArch64ISD_UZP2_MVT_v4f16_rr(RetVT, Op0, Op1);
  case MVT::v8f16: return fastEmit_AArch64ISD_UZP2_MVT_v8f16_rr(RetVT, Op0, Op1);
  case MVT::v4bf16: return fastEmit_AArch64ISD_UZP2_MVT_v4bf16_rr(RetVT, Op0, Op1);
  case MVT::v8bf16: return fastEmit_AArch64ISD_UZP2_MVT_v8bf16_rr(RetVT, Op0, Op1);
  case MVT::v2f32: return fastEmit_AArch64ISD_UZP2_MVT_v2f32_rr(RetVT, Op0, Op1);
  case MVT::v4f32: return fastEmit_AArch64ISD_UZP2_MVT_v4f32_rr(RetVT, Op0, Op1);
  case MVT::v2f64: return fastEmit_AArch64ISD_UZP2_MVT_v2f64_rr(RetVT, Op0, Op1);
  case MVT::nxv2i1: return fastEmit_AArch64ISD_UZP2_MVT_nxv2i1_rr(RetVT, Op0, Op1);
  case MVT::nxv4i1: return fastEmit_AArch64ISD_UZP2_MVT_nxv4i1_rr(RetVT, Op0, Op1);
  case MVT::nxv8i1: return fastEmit_AArch64ISD_UZP2_MVT_nxv8i1_rr(RetVT, Op0, Op1);
  case MVT::nxv16i1: return fastEmit_AArch64ISD_UZP2_MVT_nxv16i1_rr(RetVT, Op0, Op1);
  case MVT::nxv16i8: return fastEmit_AArch64ISD_UZP2_MVT_nxv16i8_rr(RetVT, Op0, Op1);
  case MVT::nxv8i16: return fastEmit_AArch64ISD_UZP2_MVT_nxv8i16_rr(RetVT, Op0, Op1);
  case MVT::nxv4i32: return fastEmit_AArch64ISD_UZP2_MVT_nxv4i32_rr(RetVT, Op0, Op1);
  case MVT::nxv2i64: return fastEmit_AArch64ISD_UZP2_MVT_nxv2i64_rr(RetVT, Op0, Op1);
  case MVT::nxv2f16: return fastEmit_AArch64ISD_UZP2_MVT_nxv2f16_rr(RetVT, Op0, Op1);
  case MVT::nxv4f16: return fastEmit_AArch64ISD_UZP2_MVT_nxv4f16_rr(RetVT, Op0, Op1);
  case MVT::nxv8f16: return fastEmit_AArch64ISD_UZP2_MVT_nxv8f16_rr(RetVT, Op0, Op1);
  case MVT::nxv8bf16: return fastEmit_AArch64ISD_UZP2_MVT_nxv8bf16_rr(RetVT, Op0, Op1);
  case MVT::nxv2f32: return fastEmit_AArch64ISD_UZP2_MVT_nxv2f32_rr(RetVT, Op0, Op1);
  case MVT::nxv4f32: return fastEmit_AArch64ISD_UZP2_MVT_nxv4f32_rr(RetVT, Op0, Op1);
  case MVT::nxv2f64: return fastEmit_AArch64ISD_UZP2_MVT_nxv2f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::ZIP1.

unsigned fastEmit_AArch64ISD_ZIP1_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ZIP1v8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP1_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ZIP1v16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP1_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ZIP1v4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP1_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ZIP1v8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP1_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ZIP1v2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP1_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ZIP1v4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP1_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ZIP1v2i64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP1_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ZIP1v4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP1_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ZIP1v8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP1_MVT_v4bf16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4bf16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ZIP1v4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP1_MVT_v8bf16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8bf16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ZIP1v8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP1_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ZIP1v2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP1_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ZIP1v4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP1_MVT_v2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ZIP1v2i64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP1_MVT_nxv2i1_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ZIP1_PPP_D, &AArch64::PPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP1_MVT_nxv4i1_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ZIP1_PPP_S, &AArch64::PPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP1_MVT_nxv8i1_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ZIP1_PPP_H, &AArch64::PPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP1_MVT_nxv16i1_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv16i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ZIP1_PPP_B, &AArch64::PPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP1_MVT_nxv16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv16i8)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ZIP1_ZZZ_B, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP1_MVT_nxv8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8i16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ZIP1_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP1_MVT_nxv4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4i32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ZIP1_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP1_MVT_nxv2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2i64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ZIP1_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP1_MVT_nxv2f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2f16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ZIP1_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP1_MVT_nxv4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4f16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ZIP1_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP1_MVT_nxv8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8f16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ZIP1_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP1_MVT_nxv8bf16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8bf16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ZIP1_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP1_MVT_nxv2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2f32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ZIP1_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP1_MVT_nxv4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4f32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ZIP1_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP1_MVT_nxv2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2f64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ZIP1_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP1_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_ZIP1_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_AArch64ISD_ZIP1_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_AArch64ISD_ZIP1_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_AArch64ISD_ZIP1_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_AArch64ISD_ZIP1_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_AArch64ISD_ZIP1_MVT_v4i32_rr(RetVT, Op0, Op1);
  case MVT::v2i64: return fastEmit_AArch64ISD_ZIP1_MVT_v2i64_rr(RetVT, Op0, Op1);
  case MVT::v4f16: return fastEmit_AArch64ISD_ZIP1_MVT_v4f16_rr(RetVT, Op0, Op1);
  case MVT::v8f16: return fastEmit_AArch64ISD_ZIP1_MVT_v8f16_rr(RetVT, Op0, Op1);
  case MVT::v4bf16: return fastEmit_AArch64ISD_ZIP1_MVT_v4bf16_rr(RetVT, Op0, Op1);
  case MVT::v8bf16: return fastEmit_AArch64ISD_ZIP1_MVT_v8bf16_rr(RetVT, Op0, Op1);
  case MVT::v2f32: return fastEmit_AArch64ISD_ZIP1_MVT_v2f32_rr(RetVT, Op0, Op1);
  case MVT::v4f32: return fastEmit_AArch64ISD_ZIP1_MVT_v4f32_rr(RetVT, Op0, Op1);
  case MVT::v2f64: return fastEmit_AArch64ISD_ZIP1_MVT_v2f64_rr(RetVT, Op0, Op1);
  case MVT::nxv2i1: return fastEmit_AArch64ISD_ZIP1_MVT_nxv2i1_rr(RetVT, Op0, Op1);
  case MVT::nxv4i1: return fastEmit_AArch64ISD_ZIP1_MVT_nxv4i1_rr(RetVT, Op0, Op1);
  case MVT::nxv8i1: return fastEmit_AArch64ISD_ZIP1_MVT_nxv8i1_rr(RetVT, Op0, Op1);
  case MVT::nxv16i1: return fastEmit_AArch64ISD_ZIP1_MVT_nxv16i1_rr(RetVT, Op0, Op1);
  case MVT::nxv16i8: return fastEmit_AArch64ISD_ZIP1_MVT_nxv16i8_rr(RetVT, Op0, Op1);
  case MVT::nxv8i16: return fastEmit_AArch64ISD_ZIP1_MVT_nxv8i16_rr(RetVT, Op0, Op1);
  case MVT::nxv4i32: return fastEmit_AArch64ISD_ZIP1_MVT_nxv4i32_rr(RetVT, Op0, Op1);
  case MVT::nxv2i64: return fastEmit_AArch64ISD_ZIP1_MVT_nxv2i64_rr(RetVT, Op0, Op1);
  case MVT::nxv2f16: return fastEmit_AArch64ISD_ZIP1_MVT_nxv2f16_rr(RetVT, Op0, Op1);
  case MVT::nxv4f16: return fastEmit_AArch64ISD_ZIP1_MVT_nxv4f16_rr(RetVT, Op0, Op1);
  case MVT::nxv8f16: return fastEmit_AArch64ISD_ZIP1_MVT_nxv8f16_rr(RetVT, Op0, Op1);
  case MVT::nxv8bf16: return fastEmit_AArch64ISD_ZIP1_MVT_nxv8bf16_rr(RetVT, Op0, Op1);
  case MVT::nxv2f32: return fastEmit_AArch64ISD_ZIP1_MVT_nxv2f32_rr(RetVT, Op0, Op1);
  case MVT::nxv4f32: return fastEmit_AArch64ISD_ZIP1_MVT_nxv4f32_rr(RetVT, Op0, Op1);
  case MVT::nxv2f64: return fastEmit_AArch64ISD_ZIP1_MVT_nxv2f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::ZIP2.

unsigned fastEmit_AArch64ISD_ZIP2_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ZIP2v8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP2_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ZIP2v16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP2_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ZIP2v4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP2_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ZIP2v8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP2_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ZIP2v2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP2_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ZIP2v4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP2_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ZIP2v2i64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP2_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ZIP2v4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP2_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ZIP2v8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP2_MVT_v4bf16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4bf16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ZIP2v4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP2_MVT_v8bf16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8bf16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ZIP2v8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP2_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ZIP2v2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP2_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ZIP2v4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP2_MVT_v2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ZIP2v2i64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP2_MVT_nxv2i1_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ZIP2_PPP_D, &AArch64::PPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP2_MVT_nxv4i1_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ZIP2_PPP_S, &AArch64::PPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP2_MVT_nxv8i1_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ZIP2_PPP_H, &AArch64::PPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP2_MVT_nxv16i1_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv16i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ZIP2_PPP_B, &AArch64::PPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP2_MVT_nxv16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv16i8)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ZIP2_ZZZ_B, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP2_MVT_nxv8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8i16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ZIP2_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP2_MVT_nxv4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4i32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ZIP2_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP2_MVT_nxv2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2i64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ZIP2_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP2_MVT_nxv2f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2f16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ZIP2_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP2_MVT_nxv4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4f16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ZIP2_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP2_MVT_nxv8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8f16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ZIP2_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP2_MVT_nxv8bf16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8bf16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ZIP2_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP2_MVT_nxv2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2f32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ZIP2_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP2_MVT_nxv4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4f32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ZIP2_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP2_MVT_nxv2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2f64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ZIP2_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_ZIP2_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_ZIP2_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_AArch64ISD_ZIP2_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_AArch64ISD_ZIP2_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_AArch64ISD_ZIP2_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_AArch64ISD_ZIP2_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_AArch64ISD_ZIP2_MVT_v4i32_rr(RetVT, Op0, Op1);
  case MVT::v2i64: return fastEmit_AArch64ISD_ZIP2_MVT_v2i64_rr(RetVT, Op0, Op1);
  case MVT::v4f16: return fastEmit_AArch64ISD_ZIP2_MVT_v4f16_rr(RetVT, Op0, Op1);
  case MVT::v8f16: return fastEmit_AArch64ISD_ZIP2_MVT_v8f16_rr(RetVT, Op0, Op1);
  case MVT::v4bf16: return fastEmit_AArch64ISD_ZIP2_MVT_v4bf16_rr(RetVT, Op0, Op1);
  case MVT::v8bf16: return fastEmit_AArch64ISD_ZIP2_MVT_v8bf16_rr(RetVT, Op0, Op1);
  case MVT::v2f32: return fastEmit_AArch64ISD_ZIP2_MVT_v2f32_rr(RetVT, Op0, Op1);
  case MVT::v4f32: return fastEmit_AArch64ISD_ZIP2_MVT_v4f32_rr(RetVT, Op0, Op1);
  case MVT::v2f64: return fastEmit_AArch64ISD_ZIP2_MVT_v2f64_rr(RetVT, Op0, Op1);
  case MVT::nxv2i1: return fastEmit_AArch64ISD_ZIP2_MVT_nxv2i1_rr(RetVT, Op0, Op1);
  case MVT::nxv4i1: return fastEmit_AArch64ISD_ZIP2_MVT_nxv4i1_rr(RetVT, Op0, Op1);
  case MVT::nxv8i1: return fastEmit_AArch64ISD_ZIP2_MVT_nxv8i1_rr(RetVT, Op0, Op1);
  case MVT::nxv16i1: return fastEmit_AArch64ISD_ZIP2_MVT_nxv16i1_rr(RetVT, Op0, Op1);
  case MVT::nxv16i8: return fastEmit_AArch64ISD_ZIP2_MVT_nxv16i8_rr(RetVT, Op0, Op1);
  case MVT::nxv8i16: return fastEmit_AArch64ISD_ZIP2_MVT_nxv8i16_rr(RetVT, Op0, Op1);
  case MVT::nxv4i32: return fastEmit_AArch64ISD_ZIP2_MVT_nxv4i32_rr(RetVT, Op0, Op1);
  case MVT::nxv2i64: return fastEmit_AArch64ISD_ZIP2_MVT_nxv2i64_rr(RetVT, Op0, Op1);
  case MVT::nxv2f16: return fastEmit_AArch64ISD_ZIP2_MVT_nxv2f16_rr(RetVT, Op0, Op1);
  case MVT::nxv4f16: return fastEmit_AArch64ISD_ZIP2_MVT_nxv4f16_rr(RetVT, Op0, Op1);
  case MVT::nxv8f16: return fastEmit_AArch64ISD_ZIP2_MVT_nxv8f16_rr(RetVT, Op0, Op1);
  case MVT::nxv8bf16: return fastEmit_AArch64ISD_ZIP2_MVT_nxv8bf16_rr(RetVT, Op0, Op1);
  case MVT::nxv2f32: return fastEmit_AArch64ISD_ZIP2_MVT_nxv2f32_rr(RetVT, Op0, Op1);
  case MVT::nxv4f32: return fastEmit_AArch64ISD_ZIP2_MVT_nxv4f32_rr(RetVT, Op0, Op1);
  case MVT::nxv2f64: return fastEmit_AArch64ISD_ZIP2_MVT_nxv2f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::ABDS.

unsigned fastEmit_ISD_ABDS_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SABDv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ABDS_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SABDv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ABDS_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SABDv4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ABDS_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SABDv8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ABDS_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SABDv2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ABDS_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SABDv4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ABDS_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ISD_ABDS_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_ISD_ABDS_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ISD_ABDS_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_ABDS_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ISD_ABDS_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_ABDS_MVT_v4i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::ABDU.

unsigned fastEmit_ISD_ABDU_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UABDv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ABDU_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UABDv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ABDU_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UABDv4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ABDU_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UABDv8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ABDU_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UABDv2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ABDU_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UABDv4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ABDU_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ISD_ABDU_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_ISD_ABDU_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ISD_ABDU_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_ABDU_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ISD_ABDU_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_ABDU_MVT_v4i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::ADD.

unsigned fastEmit_ISD_ADD_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(AArch64::ADDWrr, &AArch64::GPR32RegClass, Op0, Op1);
}

unsigned fastEmit_ISD_ADD_MVT_i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_rr(AArch64::ADDXrr, &AArch64::GPR64RegClass, Op0, Op1);
}

unsigned fastEmit_ISD_ADD_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ADDv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ADDv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ADDv4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ADDv8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ADDv2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ADDv4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v1i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ADDv1i64, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ADDv2i64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_nxv16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv16i8)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ADD_ZZZ_B, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_nxv8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8i16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ADD_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_nxv4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4i32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ADD_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_nxv2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2i64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ADD_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_ADD_MVT_i32_rr(RetVT, Op0, Op1);
  case MVT::i64: return fastEmit_ISD_ADD_MVT_i64_rr(RetVT, Op0, Op1);
  case MVT::v8i8: return fastEmit_ISD_ADD_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_ISD_ADD_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ISD_ADD_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_ADD_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ISD_ADD_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_ADD_MVT_v4i32_rr(RetVT, Op0, Op1);
  case MVT::v1i64: return fastEmit_ISD_ADD_MVT_v1i64_rr(RetVT, Op0, Op1);
  case MVT::v2i64: return fastEmit_ISD_ADD_MVT_v2i64_rr(RetVT, Op0, Op1);
  case MVT::nxv16i8: return fastEmit_ISD_ADD_MVT_nxv16i8_rr(RetVT, Op0, Op1);
  case MVT::nxv8i16: return fastEmit_ISD_ADD_MVT_nxv8i16_rr(RetVT, Op0, Op1);
  case MVT::nxv4i32: return fastEmit_ISD_ADD_MVT_nxv4i32_rr(RetVT, Op0, Op1);
  case MVT::nxv2i64: return fastEmit_ISD_ADD_MVT_nxv2i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::AND.

unsigned fastEmit_ISD_AND_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(AArch64::ANDWrr, &AArch64::GPR32RegClass, Op0, Op1);
}

unsigned fastEmit_ISD_AND_MVT_i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_rr(AArch64::ANDXrr, &AArch64::GPR64RegClass, Op0, Op1);
}

unsigned fastEmit_ISD_AND_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ANDv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ANDv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ANDv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ANDv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ANDv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ANDv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v1i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ANDv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ANDv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_nxv16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv16i8)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::AND_ZZZ, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_nxv8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8i16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::AND_ZZZ, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_nxv4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4i32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::AND_ZZZ, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_nxv2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2i64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::AND_ZZZ, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_AND_MVT_i32_rr(RetVT, Op0, Op1);
  case MVT::i64: return fastEmit_ISD_AND_MVT_i64_rr(RetVT, Op0, Op1);
  case MVT::v8i8: return fastEmit_ISD_AND_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_ISD_AND_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ISD_AND_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_AND_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ISD_AND_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_AND_MVT_v4i32_rr(RetVT, Op0, Op1);
  case MVT::v1i64: return fastEmit_ISD_AND_MVT_v1i64_rr(RetVT, Op0, Op1);
  case MVT::v2i64: return fastEmit_ISD_AND_MVT_v2i64_rr(RetVT, Op0, Op1);
  case MVT::nxv16i8: return fastEmit_ISD_AND_MVT_nxv16i8_rr(RetVT, Op0, Op1);
  case MVT::nxv8i16: return fastEmit_ISD_AND_MVT_nxv8i16_rr(RetVT, Op0, Op1);
  case MVT::nxv4i32: return fastEmit_ISD_AND_MVT_nxv4i32_rr(RetVT, Op0, Op1);
  case MVT::nxv2i64: return fastEmit_ISD_AND_MVT_nxv2i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::AVGCEILS.

unsigned fastEmit_ISD_AVGCEILS_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SRHADDv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AVGCEILS_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SRHADDv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AVGCEILS_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SRHADDv4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AVGCEILS_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SRHADDv8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AVGCEILS_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SRHADDv2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AVGCEILS_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SRHADDv4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AVGCEILS_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ISD_AVGCEILS_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_ISD_AVGCEILS_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ISD_AVGCEILS_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_AVGCEILS_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ISD_AVGCEILS_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_AVGCEILS_MVT_v4i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::AVGCEILU.

unsigned fastEmit_ISD_AVGCEILU_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::URHADDv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AVGCEILU_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::URHADDv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AVGCEILU_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::URHADDv4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AVGCEILU_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::URHADDv8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AVGCEILU_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::URHADDv2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AVGCEILU_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::URHADDv4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AVGCEILU_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ISD_AVGCEILU_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_ISD_AVGCEILU_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ISD_AVGCEILU_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_AVGCEILU_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ISD_AVGCEILU_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_AVGCEILU_MVT_v4i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::AVGFLOORS.

unsigned fastEmit_ISD_AVGFLOORS_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SHADDv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AVGFLOORS_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SHADDv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AVGFLOORS_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SHADDv4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AVGFLOORS_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SHADDv8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AVGFLOORS_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SHADDv2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AVGFLOORS_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SHADDv4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AVGFLOORS_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ISD_AVGFLOORS_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_ISD_AVGFLOORS_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ISD_AVGFLOORS_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_AVGFLOORS_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ISD_AVGFLOORS_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_AVGFLOORS_MVT_v4i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::AVGFLOORU.

unsigned fastEmit_ISD_AVGFLOORU_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UHADDv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AVGFLOORU_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UHADDv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AVGFLOORU_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UHADDv4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AVGFLOORU_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UHADDv8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AVGFLOORU_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UHADDv2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AVGFLOORU_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UHADDv4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_AVGFLOORU_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ISD_AVGFLOORU_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_ISD_AVGFLOORU_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ISD_AVGFLOORU_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_AVGFLOORU_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ISD_AVGFLOORU_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_AVGFLOORU_MVT_v4i32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::CONCAT_VECTORS.

unsigned fastEmit_ISD_CONCAT_VECTORS_MVT_nxv1i1_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP1_PPP_D, &AArch64::PPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_CONCAT_VECTORS_MVT_nxv2i1_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP1_PPP_S, &AArch64::PPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_CONCAT_VECTORS_MVT_nxv4i1_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP1_PPP_H, &AArch64::PPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_CONCAT_VECTORS_MVT_nxv8i1_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv16i1)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP1_PPP_B, &AArch64::PPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_CONCAT_VECTORS_MVT_nxv2f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4f16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP1_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_CONCAT_VECTORS_MVT_nxv4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8f16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP1_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_CONCAT_VECTORS_MVT_nxv2bf16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4bf16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP1_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_CONCAT_VECTORS_MVT_nxv4bf16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8bf16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP1_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_CONCAT_VECTORS_MVT_nxv2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4f32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UZP1_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_CONCAT_VECTORS_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::nxv1i1: return fastEmit_ISD_CONCAT_VECTORS_MVT_nxv1i1_rr(RetVT, Op0, Op1);
  case MVT::nxv2i1: return fastEmit_ISD_CONCAT_VECTORS_MVT_nxv2i1_rr(RetVT, Op0, Op1);
  case MVT::nxv4i1: return fastEmit_ISD_CONCAT_VECTORS_MVT_nxv4i1_rr(RetVT, Op0, Op1);
  case MVT::nxv8i1: return fastEmit_ISD_CONCAT_VECTORS_MVT_nxv8i1_rr(RetVT, Op0, Op1);
  case MVT::nxv2f16: return fastEmit_ISD_CONCAT_VECTORS_MVT_nxv2f16_rr(RetVT, Op0, Op1);
  case MVT::nxv4f16: return fastEmit_ISD_CONCAT_VECTORS_MVT_nxv4f16_rr(RetVT, Op0, Op1);
  case MVT::nxv2bf16: return fastEmit_ISD_CONCAT_VECTORS_MVT_nxv2bf16_rr(RetVT, Op0, Op1);
  case MVT::nxv4bf16: return fastEmit_ISD_CONCAT_VECTORS_MVT_nxv4bf16_rr(RetVT, Op0, Op1);
  case MVT::nxv2f32: return fastEmit_ISD_CONCAT_VECTORS_MVT_nxv2f32_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::FADD.

unsigned fastEmit_ISD_FADD_MVT_f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_rr(AArch64::FADDHrr, &AArch64::FPR16RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FADDSrr, &AArch64::FPR32RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FADDDrr, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FADDv4f16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FADDv8f16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FADDv2f32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FADDv4f32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_MVT_v2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FADDv2f64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_MVT_nxv8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8f16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::FADD_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_MVT_nxv4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4f32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::FADD_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_MVT_nxv2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2f64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::FADD_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
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
  case MVT::v2f64: return fastEmit_ISD_FADD_MVT_v2f64_rr(RetVT, Op0, Op1);
  case MVT::nxv8f16: return fastEmit_ISD_FADD_MVT_nxv8f16_rr(RetVT, Op0, Op1);
  case MVT::nxv4f32: return fastEmit_ISD_FADD_MVT_nxv4f32_rr(RetVT, Op0, Op1);
  case MVT::nxv2f64: return fastEmit_ISD_FADD_MVT_nxv2f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::FDIV.

unsigned fastEmit_ISD_FDIV_MVT_f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_rr(AArch64::FDIVHrr, &AArch64::FPR16RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FDIV_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FDIVSrr, &AArch64::FPR32RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FDIV_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FDIVDrr, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FDIV_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FDIVv4f16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FDIV_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FDIVv8f16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FDIV_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FDIVv2f32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FDIV_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FDIVv4f32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FDIV_MVT_v2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FDIVv2f64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FDIV_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FDIV_MVT_f16_rr(RetVT, Op0, Op1);
  case MVT::f32: return fastEmit_ISD_FDIV_MVT_f32_rr(RetVT, Op0, Op1);
  case MVT::f64: return fastEmit_ISD_FDIV_MVT_f64_rr(RetVT, Op0, Op1);
  case MVT::v4f16: return fastEmit_ISD_FDIV_MVT_v4f16_rr(RetVT, Op0, Op1);
  case MVT::v8f16: return fastEmit_ISD_FDIV_MVT_v8f16_rr(RetVT, Op0, Op1);
  case MVT::v2f32: return fastEmit_ISD_FDIV_MVT_v2f32_rr(RetVT, Op0, Op1);
  case MVT::v4f32: return fastEmit_ISD_FDIV_MVT_v4f32_rr(RetVT, Op0, Op1);
  case MVT::v2f64: return fastEmit_ISD_FDIV_MVT_v2f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::FMAXIMUM.

unsigned fastEmit_ISD_FMAXIMUM_MVT_f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_rr(AArch64::FMAXHrr, &AArch64::FPR16RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMAXIMUM_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FMAXSrr, &AArch64::FPR32RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMAXIMUM_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FMAXDrr, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMAXIMUM_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMAXv4f16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMAXIMUM_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMAXv8f16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMAXIMUM_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMAXv2f32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMAXIMUM_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMAXv4f32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMAXIMUM_MVT_v1f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v1f64)
    return 0;
  return fastEmitInst_rr(AArch64::FMAXDrr, &AArch64::FPR64RegClass, Op0, Op1);
}

unsigned fastEmit_ISD_FMAXIMUM_MVT_v2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMAXv2f64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMAXIMUM_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FMAXIMUM_MVT_f16_rr(RetVT, Op0, Op1);
  case MVT::f32: return fastEmit_ISD_FMAXIMUM_MVT_f32_rr(RetVT, Op0, Op1);
  case MVT::f64: return fastEmit_ISD_FMAXIMUM_MVT_f64_rr(RetVT, Op0, Op1);
  case MVT::v4f16: return fastEmit_ISD_FMAXIMUM_MVT_v4f16_rr(RetVT, Op0, Op1);
  case MVT::v8f16: return fastEmit_ISD_FMAXIMUM_MVT_v8f16_rr(RetVT, Op0, Op1);
  case MVT::v2f32: return fastEmit_ISD_FMAXIMUM_MVT_v2f32_rr(RetVT, Op0, Op1);
  case MVT::v4f32: return fastEmit_ISD_FMAXIMUM_MVT_v4f32_rr(RetVT, Op0, Op1);
  case MVT::v1f64: return fastEmit_ISD_FMAXIMUM_MVT_v1f64_rr(RetVT, Op0, Op1);
  case MVT::v2f64: return fastEmit_ISD_FMAXIMUM_MVT_v2f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::FMAXNUM.

unsigned fastEmit_ISD_FMAXNUM_MVT_f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_rr(AArch64::FMAXNMHrr, &AArch64::FPR16RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMAXNUM_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FMAXNMSrr, &AArch64::FPR32RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMAXNUM_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FMAXNMDrr, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMAXNUM_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMAXNMv4f16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMAXNUM_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMAXNMv8f16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMAXNUM_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMAXNMv2f32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMAXNUM_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMAXNMv4f32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMAXNUM_MVT_v1f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v1f64)
    return 0;
  return fastEmitInst_rr(AArch64::FMAXNMDrr, &AArch64::FPR64RegClass, Op0, Op1);
}

unsigned fastEmit_ISD_FMAXNUM_MVT_v2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMAXNMv2f64, &AArch64::FPR128RegClass, Op0, Op1);
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
  case MVT::v1f64: return fastEmit_ISD_FMAXNUM_MVT_v1f64_rr(RetVT, Op0, Op1);
  case MVT::v2f64: return fastEmit_ISD_FMAXNUM_MVT_v2f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::FMINIMUM.

unsigned fastEmit_ISD_FMINIMUM_MVT_f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_rr(AArch64::FMINHrr, &AArch64::FPR16RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMINIMUM_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FMINSrr, &AArch64::FPR32RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMINIMUM_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FMINDrr, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMINIMUM_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMINv4f16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMINIMUM_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMINv8f16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMINIMUM_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMINv2f32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMINIMUM_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMINv4f32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMINIMUM_MVT_v1f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v1f64)
    return 0;
  return fastEmitInst_rr(AArch64::FMINDrr, &AArch64::FPR64RegClass, Op0, Op1);
}

unsigned fastEmit_ISD_FMINIMUM_MVT_v2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMINv2f64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMINIMUM_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FMINIMUM_MVT_f16_rr(RetVT, Op0, Op1);
  case MVT::f32: return fastEmit_ISD_FMINIMUM_MVT_f32_rr(RetVT, Op0, Op1);
  case MVT::f64: return fastEmit_ISD_FMINIMUM_MVT_f64_rr(RetVT, Op0, Op1);
  case MVT::v4f16: return fastEmit_ISD_FMINIMUM_MVT_v4f16_rr(RetVT, Op0, Op1);
  case MVT::v8f16: return fastEmit_ISD_FMINIMUM_MVT_v8f16_rr(RetVT, Op0, Op1);
  case MVT::v2f32: return fastEmit_ISD_FMINIMUM_MVT_v2f32_rr(RetVT, Op0, Op1);
  case MVT::v4f32: return fastEmit_ISD_FMINIMUM_MVT_v4f32_rr(RetVT, Op0, Op1);
  case MVT::v1f64: return fastEmit_ISD_FMINIMUM_MVT_v1f64_rr(RetVT, Op0, Op1);
  case MVT::v2f64: return fastEmit_ISD_FMINIMUM_MVT_v2f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::FMINNUM.

unsigned fastEmit_ISD_FMINNUM_MVT_f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_rr(AArch64::FMINNMHrr, &AArch64::FPR16RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMINNUM_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FMINNMSrr, &AArch64::FPR32RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMINNUM_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FMINNMDrr, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMINNUM_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMINNMv4f16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMINNUM_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMINNMv8f16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMINNUM_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMINNMv2f32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMINNUM_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMINNMv4f32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMINNUM_MVT_v1f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v1f64)
    return 0;
  return fastEmitInst_rr(AArch64::FMINNMDrr, &AArch64::FPR64RegClass, Op0, Op1);
}

unsigned fastEmit_ISD_FMINNUM_MVT_v2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMINNMv2f64, &AArch64::FPR128RegClass, Op0, Op1);
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
  case MVT::v1f64: return fastEmit_ISD_FMINNUM_MVT_v1f64_rr(RetVT, Op0, Op1);
  case MVT::v2f64: return fastEmit_ISD_FMINNUM_MVT_v2f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::FMUL.

unsigned fastEmit_ISD_FMUL_MVT_f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_rr(AArch64::FMULHrr, &AArch64::FPR16RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FMULSrr, &AArch64::FPR32RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FMULDrr, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMULv4f16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMULv8f16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMULv2f32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMULv4f32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_MVT_v2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMULv2f64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_MVT_nxv8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8f16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::FMUL_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_MVT_nxv4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4f32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::FMUL_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_MVT_nxv2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2f64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::FMUL_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
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
  case MVT::v2f64: return fastEmit_ISD_FMUL_MVT_v2f64_rr(RetVT, Op0, Op1);
  case MVT::nxv8f16: return fastEmit_ISD_FMUL_MVT_nxv8f16_rr(RetVT, Op0, Op1);
  case MVT::nxv4f32: return fastEmit_ISD_FMUL_MVT_nxv4f32_rr(RetVT, Op0, Op1);
  case MVT::nxv2f64: return fastEmit_ISD_FMUL_MVT_nxv2f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::FSUB.

unsigned fastEmit_ISD_FSUB_MVT_f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_rr(AArch64::FSUBHrr, &AArch64::FPR16RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FSUBSrr, &AArch64::FPR32RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FSUBDrr, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FSUBv4f16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FSUBv8f16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FSUBv2f32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FSUBv4f32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_MVT_v2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FSUBv2f64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_MVT_nxv8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8f16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::FSUB_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_MVT_nxv4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4f32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::FSUB_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_MVT_nxv2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2f64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::FSUB_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
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
  case MVT::v2f64: return fastEmit_ISD_FSUB_MVT_v2f64_rr(RetVT, Op0, Op1);
  case MVT::nxv8f16: return fastEmit_ISD_FSUB_MVT_nxv8f16_rr(RetVT, Op0, Op1);
  case MVT::nxv4f32: return fastEmit_ISD_FSUB_MVT_nxv4f32_rr(RetVT, Op0, Op1);
  case MVT::nxv2f64: return fastEmit_ISD_FSUB_MVT_nxv2f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::MUL.

unsigned fastEmit_ISD_MUL_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::MULv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_MUL_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::MULv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_MUL_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::MULv4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_MUL_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::MULv8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_MUL_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::MULv2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_MUL_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::MULv4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_MUL_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
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

unsigned fastEmit_ISD_MULHS_MVT_i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_rr(AArch64::SMULHrr, &AArch64::GPR64RegClass, Op0, Op1);
}

unsigned fastEmit_ISD_MULHS_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_MULHS_MVT_i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::MULHU.

unsigned fastEmit_ISD_MULHU_MVT_i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_rr(AArch64::UMULHrr, &AArch64::GPR64RegClass, Op0, Op1);
}

unsigned fastEmit_ISD_MULHU_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_MULHU_MVT_i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::OR.

unsigned fastEmit_ISD_OR_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(AArch64::ORRWrr, &AArch64::GPR32RegClass, Op0, Op1);
}

unsigned fastEmit_ISD_OR_MVT_i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_rr(AArch64::ORRXrr, &AArch64::GPR64RegClass, Op0, Op1);
}

unsigned fastEmit_ISD_OR_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ORRv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ORRv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ORRv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ORRv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ORRv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ORRv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v1i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ORRv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::ORRv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_nxv16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv16i8)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ORR_ZZZ, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_nxv8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8i16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ORR_ZZZ, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_nxv4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4i32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ORR_ZZZ, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_nxv2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2i64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::ORR_ZZZ, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_OR_MVT_i32_rr(RetVT, Op0, Op1);
  case MVT::i64: return fastEmit_ISD_OR_MVT_i64_rr(RetVT, Op0, Op1);
  case MVT::v8i8: return fastEmit_ISD_OR_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_ISD_OR_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ISD_OR_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_OR_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ISD_OR_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_OR_MVT_v4i32_rr(RetVT, Op0, Op1);
  case MVT::v1i64: return fastEmit_ISD_OR_MVT_v1i64_rr(RetVT, Op0, Op1);
  case MVT::v2i64: return fastEmit_ISD_OR_MVT_v2i64_rr(RetVT, Op0, Op1);
  case MVT::nxv16i8: return fastEmit_ISD_OR_MVT_nxv16i8_rr(RetVT, Op0, Op1);
  case MVT::nxv8i16: return fastEmit_ISD_OR_MVT_nxv8i16_rr(RetVT, Op0, Op1);
  case MVT::nxv4i32: return fastEmit_ISD_OR_MVT_nxv4i32_rr(RetVT, Op0, Op1);
  case MVT::nxv2i64: return fastEmit_ISD_OR_MVT_nxv2i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::ROTR.

unsigned fastEmit_ISD_ROTR_MVT_i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_rr(AArch64::RORVXr, &AArch64::GPR64RegClass, Op0, Op1);
}

unsigned fastEmit_ISD_ROTR_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_ROTR_MVT_i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SADDSAT.

unsigned fastEmit_ISD_SADDSAT_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SQADDv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SADDSAT_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SQADDv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SADDSAT_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SQADDv4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SADDSAT_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SQADDv8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SADDSAT_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SQADDv2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SADDSAT_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SQADDv4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SADDSAT_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SQADDv2i64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SADDSAT_MVT_nxv16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv16i8)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::SQADD_ZZZ_B, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SADDSAT_MVT_nxv8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8i16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::SQADD_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SADDSAT_MVT_nxv4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4i32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::SQADD_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SADDSAT_MVT_nxv2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2i64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::SQADD_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SADDSAT_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ISD_SADDSAT_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_ISD_SADDSAT_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ISD_SADDSAT_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_SADDSAT_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ISD_SADDSAT_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_SADDSAT_MVT_v4i32_rr(RetVT, Op0, Op1);
  case MVT::v2i64: return fastEmit_ISD_SADDSAT_MVT_v2i64_rr(RetVT, Op0, Op1);
  case MVT::nxv16i8: return fastEmit_ISD_SADDSAT_MVT_nxv16i8_rr(RetVT, Op0, Op1);
  case MVT::nxv8i16: return fastEmit_ISD_SADDSAT_MVT_nxv8i16_rr(RetVT, Op0, Op1);
  case MVT::nxv4i32: return fastEmit_ISD_SADDSAT_MVT_nxv4i32_rr(RetVT, Op0, Op1);
  case MVT::nxv2i64: return fastEmit_ISD_SADDSAT_MVT_nxv2i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SDIV.

unsigned fastEmit_ISD_SDIV_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(AArch64::SDIVWr, &AArch64::GPR32RegClass, Op0, Op1);
}

unsigned fastEmit_ISD_SDIV_MVT_i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_rr(AArch64::SDIVXr, &AArch64::GPR64RegClass, Op0, Op1);
}

unsigned fastEmit_ISD_SDIV_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SDIV_MVT_i32_rr(RetVT, Op0, Op1);
  case MVT::i64: return fastEmit_ISD_SDIV_MVT_i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SHL.

unsigned fastEmit_ISD_SHL_MVT_i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_rr(AArch64::LSLVXr, &AArch64::GPR64RegClass, Op0, Op1);
}

unsigned fastEmit_ISD_SHL_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_SHL_MVT_i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SMAX.

unsigned fastEmit_ISD_SMAX_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasCSSC())) {
    return fastEmitInst_rr(AArch64::SMAXWrr, &AArch64::GPR32RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_MVT_i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasCSSC())) {
    return fastEmitInst_rr(AArch64::SMAXXrr, &AArch64::GPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SMAXv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SMAXv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SMAXv4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SMAXv8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SMAXv2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SMAXv4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SMAX_MVT_i32_rr(RetVT, Op0, Op1);
  case MVT::i64: return fastEmit_ISD_SMAX_MVT_i64_rr(RetVT, Op0, Op1);
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

unsigned fastEmit_ISD_SMIN_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasCSSC())) {
    return fastEmitInst_rr(AArch64::SMINWrr, &AArch64::GPR32RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_MVT_i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasCSSC())) {
    return fastEmitInst_rr(AArch64::SMINXrr, &AArch64::GPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SMINv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SMINv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SMINv4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SMINv8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SMINv2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SMINv4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SMIN_MVT_i32_rr(RetVT, Op0, Op1);
  case MVT::i64: return fastEmit_ISD_SMIN_MVT_i64_rr(RetVT, Op0, Op1);
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

unsigned fastEmit_ISD_SRA_MVT_i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_rr(AArch64::ASRVXr, &AArch64::GPR64RegClass, Op0, Op1);
}

unsigned fastEmit_ISD_SRA_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_SRA_MVT_i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SRL.

unsigned fastEmit_ISD_SRL_MVT_i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_rr(AArch64::LSRVXr, &AArch64::GPR64RegClass, Op0, Op1);
}

unsigned fastEmit_ISD_SRL_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_SRL_MVT_i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SSUBSAT.

unsigned fastEmit_ISD_SSUBSAT_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SQSUBv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SSUBSAT_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SQSUBv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SSUBSAT_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SQSUBv4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SSUBSAT_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SQSUBv8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SSUBSAT_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SQSUBv2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SSUBSAT_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SQSUBv4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SSUBSAT_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SQSUBv2i64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SSUBSAT_MVT_nxv16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv16i8)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::SQSUB_ZZZ_B, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SSUBSAT_MVT_nxv8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8i16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::SQSUB_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SSUBSAT_MVT_nxv4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4i32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::SQSUB_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SSUBSAT_MVT_nxv2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2i64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::SQSUB_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SSUBSAT_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_ISD_SSUBSAT_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_ISD_SSUBSAT_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ISD_SSUBSAT_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_SSUBSAT_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ISD_SSUBSAT_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_SSUBSAT_MVT_v4i32_rr(RetVT, Op0, Op1);
  case MVT::v2i64: return fastEmit_ISD_SSUBSAT_MVT_v2i64_rr(RetVT, Op0, Op1);
  case MVT::nxv16i8: return fastEmit_ISD_SSUBSAT_MVT_nxv16i8_rr(RetVT, Op0, Op1);
  case MVT::nxv8i16: return fastEmit_ISD_SSUBSAT_MVT_nxv8i16_rr(RetVT, Op0, Op1);
  case MVT::nxv4i32: return fastEmit_ISD_SSUBSAT_MVT_nxv4i32_rr(RetVT, Op0, Op1);
  case MVT::nxv2i64: return fastEmit_ISD_SSUBSAT_MVT_nxv2i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_FADD.

unsigned fastEmit_ISD_STRICT_FADD_MVT_f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_rr(AArch64::FADDHrr, &AArch64::FPR16RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FADD_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FADDSrr, &AArch64::FPR32RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FADD_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FADDDrr, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FADD_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FADDv4f16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FADD_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FADDv8f16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FADD_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FADDv2f32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FADD_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FADDv4f32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FADD_MVT_v2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FADDv2f64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FADD_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_STRICT_FADD_MVT_f16_rr(RetVT, Op0, Op1);
  case MVT::f32: return fastEmit_ISD_STRICT_FADD_MVT_f32_rr(RetVT, Op0, Op1);
  case MVT::f64: return fastEmit_ISD_STRICT_FADD_MVT_f64_rr(RetVT, Op0, Op1);
  case MVT::v4f16: return fastEmit_ISD_STRICT_FADD_MVT_v4f16_rr(RetVT, Op0, Op1);
  case MVT::v8f16: return fastEmit_ISD_STRICT_FADD_MVT_v8f16_rr(RetVT, Op0, Op1);
  case MVT::v2f32: return fastEmit_ISD_STRICT_FADD_MVT_v2f32_rr(RetVT, Op0, Op1);
  case MVT::v4f32: return fastEmit_ISD_STRICT_FADD_MVT_v4f32_rr(RetVT, Op0, Op1);
  case MVT::v2f64: return fastEmit_ISD_STRICT_FADD_MVT_v2f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_FDIV.

unsigned fastEmit_ISD_STRICT_FDIV_MVT_f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_rr(AArch64::FDIVHrr, &AArch64::FPR16RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FDIV_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FDIVSrr, &AArch64::FPR32RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FDIV_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FDIVDrr, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FDIV_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FDIVv4f16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FDIV_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FDIVv8f16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FDIV_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FDIVv2f32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FDIV_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FDIVv4f32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FDIV_MVT_v2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FDIVv2f64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FDIV_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_STRICT_FDIV_MVT_f16_rr(RetVT, Op0, Op1);
  case MVT::f32: return fastEmit_ISD_STRICT_FDIV_MVT_f32_rr(RetVT, Op0, Op1);
  case MVT::f64: return fastEmit_ISD_STRICT_FDIV_MVT_f64_rr(RetVT, Op0, Op1);
  case MVT::v4f16: return fastEmit_ISD_STRICT_FDIV_MVT_v4f16_rr(RetVT, Op0, Op1);
  case MVT::v8f16: return fastEmit_ISD_STRICT_FDIV_MVT_v8f16_rr(RetVT, Op0, Op1);
  case MVT::v2f32: return fastEmit_ISD_STRICT_FDIV_MVT_v2f32_rr(RetVT, Op0, Op1);
  case MVT::v4f32: return fastEmit_ISD_STRICT_FDIV_MVT_v4f32_rr(RetVT, Op0, Op1);
  case MVT::v2f64: return fastEmit_ISD_STRICT_FDIV_MVT_v2f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_FMAXIMUM.

unsigned fastEmit_ISD_STRICT_FMAXIMUM_MVT_f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_rr(AArch64::FMAXHrr, &AArch64::FPR16RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMAXIMUM_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FMAXSrr, &AArch64::FPR32RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMAXIMUM_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FMAXDrr, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMAXIMUM_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMAXv4f16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMAXIMUM_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMAXv8f16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMAXIMUM_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMAXv2f32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMAXIMUM_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMAXv4f32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMAXIMUM_MVT_v2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMAXv2f64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMAXIMUM_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_STRICT_FMAXIMUM_MVT_f16_rr(RetVT, Op0, Op1);
  case MVT::f32: return fastEmit_ISD_STRICT_FMAXIMUM_MVT_f32_rr(RetVT, Op0, Op1);
  case MVT::f64: return fastEmit_ISD_STRICT_FMAXIMUM_MVT_f64_rr(RetVT, Op0, Op1);
  case MVT::v4f16: return fastEmit_ISD_STRICT_FMAXIMUM_MVT_v4f16_rr(RetVT, Op0, Op1);
  case MVT::v8f16: return fastEmit_ISD_STRICT_FMAXIMUM_MVT_v8f16_rr(RetVT, Op0, Op1);
  case MVT::v2f32: return fastEmit_ISD_STRICT_FMAXIMUM_MVT_v2f32_rr(RetVT, Op0, Op1);
  case MVT::v4f32: return fastEmit_ISD_STRICT_FMAXIMUM_MVT_v4f32_rr(RetVT, Op0, Op1);
  case MVT::v2f64: return fastEmit_ISD_STRICT_FMAXIMUM_MVT_v2f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_FMAXNUM.

unsigned fastEmit_ISD_STRICT_FMAXNUM_MVT_f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_rr(AArch64::FMAXNMHrr, &AArch64::FPR16RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMAXNUM_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FMAXNMSrr, &AArch64::FPR32RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMAXNUM_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FMAXNMDrr, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMAXNUM_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMAXNMv4f16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMAXNUM_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMAXNMv8f16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMAXNUM_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMAXNMv2f32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMAXNUM_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMAXNMv4f32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMAXNUM_MVT_v2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMAXNMv2f64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMAXNUM_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_STRICT_FMAXNUM_MVT_f16_rr(RetVT, Op0, Op1);
  case MVT::f32: return fastEmit_ISD_STRICT_FMAXNUM_MVT_f32_rr(RetVT, Op0, Op1);
  case MVT::f64: return fastEmit_ISD_STRICT_FMAXNUM_MVT_f64_rr(RetVT, Op0, Op1);
  case MVT::v4f16: return fastEmit_ISD_STRICT_FMAXNUM_MVT_v4f16_rr(RetVT, Op0, Op1);
  case MVT::v8f16: return fastEmit_ISD_STRICT_FMAXNUM_MVT_v8f16_rr(RetVT, Op0, Op1);
  case MVT::v2f32: return fastEmit_ISD_STRICT_FMAXNUM_MVT_v2f32_rr(RetVT, Op0, Op1);
  case MVT::v4f32: return fastEmit_ISD_STRICT_FMAXNUM_MVT_v4f32_rr(RetVT, Op0, Op1);
  case MVT::v2f64: return fastEmit_ISD_STRICT_FMAXNUM_MVT_v2f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_FMINIMUM.

unsigned fastEmit_ISD_STRICT_FMINIMUM_MVT_f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_rr(AArch64::FMINHrr, &AArch64::FPR16RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMINIMUM_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FMINSrr, &AArch64::FPR32RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMINIMUM_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FMINDrr, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMINIMUM_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMINv4f16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMINIMUM_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMINv8f16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMINIMUM_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMINv2f32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMINIMUM_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMINv4f32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMINIMUM_MVT_v2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMINv2f64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMINIMUM_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_STRICT_FMINIMUM_MVT_f16_rr(RetVT, Op0, Op1);
  case MVT::f32: return fastEmit_ISD_STRICT_FMINIMUM_MVT_f32_rr(RetVT, Op0, Op1);
  case MVT::f64: return fastEmit_ISD_STRICT_FMINIMUM_MVT_f64_rr(RetVT, Op0, Op1);
  case MVT::v4f16: return fastEmit_ISD_STRICT_FMINIMUM_MVT_v4f16_rr(RetVT, Op0, Op1);
  case MVT::v8f16: return fastEmit_ISD_STRICT_FMINIMUM_MVT_v8f16_rr(RetVT, Op0, Op1);
  case MVT::v2f32: return fastEmit_ISD_STRICT_FMINIMUM_MVT_v2f32_rr(RetVT, Op0, Op1);
  case MVT::v4f32: return fastEmit_ISD_STRICT_FMINIMUM_MVT_v4f32_rr(RetVT, Op0, Op1);
  case MVT::v2f64: return fastEmit_ISD_STRICT_FMINIMUM_MVT_v2f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_FMINNUM.

unsigned fastEmit_ISD_STRICT_FMINNUM_MVT_f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_rr(AArch64::FMINNMHrr, &AArch64::FPR16RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMINNUM_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FMINNMSrr, &AArch64::FPR32RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMINNUM_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FMINNMDrr, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMINNUM_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMINNMv4f16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMINNUM_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMINNMv8f16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMINNUM_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMINNMv2f32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMINNUM_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMINNMv4f32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMINNUM_MVT_v2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMINNMv2f64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMINNUM_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_STRICT_FMINNUM_MVT_f16_rr(RetVT, Op0, Op1);
  case MVT::f32: return fastEmit_ISD_STRICT_FMINNUM_MVT_f32_rr(RetVT, Op0, Op1);
  case MVT::f64: return fastEmit_ISD_STRICT_FMINNUM_MVT_f64_rr(RetVT, Op0, Op1);
  case MVT::v4f16: return fastEmit_ISD_STRICT_FMINNUM_MVT_v4f16_rr(RetVT, Op0, Op1);
  case MVT::v8f16: return fastEmit_ISD_STRICT_FMINNUM_MVT_v8f16_rr(RetVT, Op0, Op1);
  case MVT::v2f32: return fastEmit_ISD_STRICT_FMINNUM_MVT_v2f32_rr(RetVT, Op0, Op1);
  case MVT::v4f32: return fastEmit_ISD_STRICT_FMINNUM_MVT_v4f32_rr(RetVT, Op0, Op1);
  case MVT::v2f64: return fastEmit_ISD_STRICT_FMINNUM_MVT_v2f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_FMUL.

unsigned fastEmit_ISD_STRICT_FMUL_MVT_f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_rr(AArch64::FMULHrr, &AArch64::FPR16RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMUL_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FMULSrr, &AArch64::FPR32RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMUL_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FMULDrr, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMUL_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMULv4f16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMUL_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMULv8f16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMUL_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMULv2f32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMUL_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMULv4f32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMUL_MVT_v2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FMULv2f64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMUL_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_STRICT_FMUL_MVT_f16_rr(RetVT, Op0, Op1);
  case MVT::f32: return fastEmit_ISD_STRICT_FMUL_MVT_f32_rr(RetVT, Op0, Op1);
  case MVT::f64: return fastEmit_ISD_STRICT_FMUL_MVT_f64_rr(RetVT, Op0, Op1);
  case MVT::v4f16: return fastEmit_ISD_STRICT_FMUL_MVT_v4f16_rr(RetVT, Op0, Op1);
  case MVT::v8f16: return fastEmit_ISD_STRICT_FMUL_MVT_v8f16_rr(RetVT, Op0, Op1);
  case MVT::v2f32: return fastEmit_ISD_STRICT_FMUL_MVT_v2f32_rr(RetVT, Op0, Op1);
  case MVT::v4f32: return fastEmit_ISD_STRICT_FMUL_MVT_v4f32_rr(RetVT, Op0, Op1);
  case MVT::v2f64: return fastEmit_ISD_STRICT_FMUL_MVT_v2f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_FSUB.

unsigned fastEmit_ISD_STRICT_FSUB_MVT_f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasFullFP16())) {
    return fastEmitInst_rr(AArch64::FSUBHrr, &AArch64::FPR16RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSUB_MVT_f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FSUBSrr, &AArch64::FPR32RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSUB_MVT_f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasFPARMv8())) {
    return fastEmitInst_rr(AArch64::FSUBDrr, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSUB_MVT_v4f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FSUBv4f16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSUB_MVT_v8f16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8f16)
    return 0;
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FSUBv8f16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSUB_MVT_v2f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FSUBv2f32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSUB_MVT_v4f32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FSUBv4f32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSUB_MVT_v2f64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::FSUBv2f64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSUB_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_STRICT_FSUB_MVT_f16_rr(RetVT, Op0, Op1);
  case MVT::f32: return fastEmit_ISD_STRICT_FSUB_MVT_f32_rr(RetVT, Op0, Op1);
  case MVT::f64: return fastEmit_ISD_STRICT_FSUB_MVT_f64_rr(RetVT, Op0, Op1);
  case MVT::v4f16: return fastEmit_ISD_STRICT_FSUB_MVT_v4f16_rr(RetVT, Op0, Op1);
  case MVT::v8f16: return fastEmit_ISD_STRICT_FSUB_MVT_v8f16_rr(RetVT, Op0, Op1);
  case MVT::v2f32: return fastEmit_ISD_STRICT_FSUB_MVT_v2f32_rr(RetVT, Op0, Op1);
  case MVT::v4f32: return fastEmit_ISD_STRICT_FSUB_MVT_v4f32_rr(RetVT, Op0, Op1);
  case MVT::v2f64: return fastEmit_ISD_STRICT_FSUB_MVT_v2f64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SUB.

unsigned fastEmit_ISD_SUB_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(AArch64::SUBSWrr, &AArch64::GPR32RegClass, Op0, Op1);
}

unsigned fastEmit_ISD_SUB_MVT_i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_rr(AArch64::SUBSXrr, &AArch64::GPR64RegClass, Op0, Op1);
}

unsigned fastEmit_ISD_SUB_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SUBv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SUBv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SUBv4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SUBv8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SUBv2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SUBv4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v1i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SUBv1i64, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::SUBv2i64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_nxv16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv16i8)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::SUB_ZZZ_B, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_nxv8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8i16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::SUB_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_nxv4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4i32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::SUB_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_nxv2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2i64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::SUB_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SUB_MVT_i32_rr(RetVT, Op0, Op1);
  case MVT::i64: return fastEmit_ISD_SUB_MVT_i64_rr(RetVT, Op0, Op1);
  case MVT::v8i8: return fastEmit_ISD_SUB_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_ISD_SUB_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ISD_SUB_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_SUB_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ISD_SUB_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_SUB_MVT_v4i32_rr(RetVT, Op0, Op1);
  case MVT::v1i64: return fastEmit_ISD_SUB_MVT_v1i64_rr(RetVT, Op0, Op1);
  case MVT::v2i64: return fastEmit_ISD_SUB_MVT_v2i64_rr(RetVT, Op0, Op1);
  case MVT::nxv16i8: return fastEmit_ISD_SUB_MVT_nxv16i8_rr(RetVT, Op0, Op1);
  case MVT::nxv8i16: return fastEmit_ISD_SUB_MVT_nxv8i16_rr(RetVT, Op0, Op1);
  case MVT::nxv4i32: return fastEmit_ISD_SUB_MVT_nxv4i32_rr(RetVT, Op0, Op1);
  case MVT::nxv2i64: return fastEmit_ISD_SUB_MVT_nxv2i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::UADDSAT.

unsigned fastEmit_ISD_UADDSAT_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UQADDv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UADDSAT_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UQADDv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UADDSAT_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UQADDv4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UADDSAT_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UQADDv8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UADDSAT_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UQADDv2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UADDSAT_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UQADDv4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UADDSAT_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UQADDv2i64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UADDSAT_MVT_nxv16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv16i8)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UQADD_ZZZ_B, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UADDSAT_MVT_nxv8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8i16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UQADD_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UADDSAT_MVT_nxv4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4i32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UQADD_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UADDSAT_MVT_nxv2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2i64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UQADD_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
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
  case MVT::v2i64: return fastEmit_ISD_UADDSAT_MVT_v2i64_rr(RetVT, Op0, Op1);
  case MVT::nxv16i8: return fastEmit_ISD_UADDSAT_MVT_nxv16i8_rr(RetVT, Op0, Op1);
  case MVT::nxv8i16: return fastEmit_ISD_UADDSAT_MVT_nxv8i16_rr(RetVT, Op0, Op1);
  case MVT::nxv4i32: return fastEmit_ISD_UADDSAT_MVT_nxv4i32_rr(RetVT, Op0, Op1);
  case MVT::nxv2i64: return fastEmit_ISD_UADDSAT_MVT_nxv2i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::UDIV.

unsigned fastEmit_ISD_UDIV_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(AArch64::UDIVWr, &AArch64::GPR32RegClass, Op0, Op1);
}

unsigned fastEmit_ISD_UDIV_MVT_i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_rr(AArch64::UDIVXr, &AArch64::GPR64RegClass, Op0, Op1);
}

unsigned fastEmit_ISD_UDIV_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_UDIV_MVT_i32_rr(RetVT, Op0, Op1);
  case MVT::i64: return fastEmit_ISD_UDIV_MVT_i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::UMAX.

unsigned fastEmit_ISD_UMAX_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasCSSC())) {
    return fastEmitInst_rr(AArch64::UMAXWrr, &AArch64::GPR32RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_MVT_i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasCSSC())) {
    return fastEmitInst_rr(AArch64::UMAXXrr, &AArch64::GPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UMAXv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UMAXv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UMAXv4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UMAXv8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UMAXv2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UMAXv4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_UMAX_MVT_i32_rr(RetVT, Op0, Op1);
  case MVT::i64: return fastEmit_ISD_UMAX_MVT_i64_rr(RetVT, Op0, Op1);
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

unsigned fastEmit_ISD_UMIN_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasCSSC())) {
    return fastEmitInst_rr(AArch64::UMINWrr, &AArch64::GPR32RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_MVT_i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasCSSC())) {
    return fastEmitInst_rr(AArch64::UMINXrr, &AArch64::GPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UMINv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UMINv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UMINv4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UMINv8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UMINv2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UMINv4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_UMIN_MVT_i32_rr(RetVT, Op0, Op1);
  case MVT::i64: return fastEmit_ISD_UMIN_MVT_i64_rr(RetVT, Op0, Op1);
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
    return fastEmitInst_rr(AArch64::UQSUBv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_USUBSAT_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UQSUBv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_USUBSAT_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UQSUBv4i16, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_USUBSAT_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UQSUBv8i16, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_USUBSAT_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UQSUBv2i32, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_USUBSAT_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UQSUBv4i32, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_USUBSAT_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::UQSUBv2i64, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_USUBSAT_MVT_nxv16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv16i8)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UQSUB_ZZZ_B, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_USUBSAT_MVT_nxv8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8i16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UQSUB_ZZZ_H, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_USUBSAT_MVT_nxv4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4i32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UQSUB_ZZZ_S, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_USUBSAT_MVT_nxv2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2i64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::UQSUB_ZZZ_D, &AArch64::ZPRRegClass, Op0, Op1);
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
  case MVT::v2i64: return fastEmit_ISD_USUBSAT_MVT_v2i64_rr(RetVT, Op0, Op1);
  case MVT::nxv16i8: return fastEmit_ISD_USUBSAT_MVT_nxv16i8_rr(RetVT, Op0, Op1);
  case MVT::nxv8i16: return fastEmit_ISD_USUBSAT_MVT_nxv8i16_rr(RetVT, Op0, Op1);
  case MVT::nxv4i32: return fastEmit_ISD_USUBSAT_MVT_nxv4i32_rr(RetVT, Op0, Op1);
  case MVT::nxv2i64: return fastEmit_ISD_USUBSAT_MVT_nxv2i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// FastEmit functions for ISD::XOR.

unsigned fastEmit_ISD_XOR_MVT_i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(AArch64::EORWrr, &AArch64::GPR32RegClass, Op0, Op1);
}

unsigned fastEmit_ISD_XOR_MVT_i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_rr(AArch64::EORXrr, &AArch64::GPR64RegClass, Op0, Op1);
}

unsigned fastEmit_ISD_XOR_MVT_v8i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::EORv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::EORv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v4i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::EORv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::EORv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v2i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::EORv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::EORv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v1i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::EORv8i8, &AArch64::FPR64RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_rr(AArch64::EORv16i8, &AArch64::FPR128RegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_nxv16i8_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv16i8)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::EOR_ZZZ, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_nxv8i16_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv8i16)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::EOR_ZZZ, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_nxv4i32_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv4i32)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::EOR_ZZZ, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_nxv2i64_rr(MVT RetVT, unsigned Op0, unsigned Op1) {
  if (RetVT.SimpleTy != MVT::nxv2i64)
    return 0;
  if ((Subtarget->hasSVEorSME())) {
    return fastEmitInst_rr(AArch64::EOR_ZZZ, &AArch64::ZPRRegClass, Op0, Op1);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_rr(MVT VT, MVT RetVT, unsigned Op0, unsigned Op1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_XOR_MVT_i32_rr(RetVT, Op0, Op1);
  case MVT::i64: return fastEmit_ISD_XOR_MVT_i64_rr(RetVT, Op0, Op1);
  case MVT::v8i8: return fastEmit_ISD_XOR_MVT_v8i8_rr(RetVT, Op0, Op1);
  case MVT::v16i8: return fastEmit_ISD_XOR_MVT_v16i8_rr(RetVT, Op0, Op1);
  case MVT::v4i16: return fastEmit_ISD_XOR_MVT_v4i16_rr(RetVT, Op0, Op1);
  case MVT::v8i16: return fastEmit_ISD_XOR_MVT_v8i16_rr(RetVT, Op0, Op1);
  case MVT::v2i32: return fastEmit_ISD_XOR_MVT_v2i32_rr(RetVT, Op0, Op1);
  case MVT::v4i32: return fastEmit_ISD_XOR_MVT_v4i32_rr(RetVT, Op0, Op1);
  case MVT::v1i64: return fastEmit_ISD_XOR_MVT_v1i64_rr(RetVT, Op0, Op1);
  case MVT::v2i64: return fastEmit_ISD_XOR_MVT_v2i64_rr(RetVT, Op0, Op1);
  case MVT::nxv16i8: return fastEmit_ISD_XOR_MVT_nxv16i8_rr(RetVT, Op0, Op1);
  case MVT::nxv8i16: return fastEmit_ISD_XOR_MVT_nxv8i16_rr(RetVT, Op0, Op1);
  case MVT::nxv4i32: return fastEmit_ISD_XOR_MVT_nxv4i32_rr(RetVT, Op0, Op1);
  case MVT::nxv2i64: return fastEmit_ISD_XOR_MVT_nxv2i64_rr(RetVT, Op0, Op1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_rr(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, unsigned Op1) override {
  switch (Opcode) {
  case AArch64ISD::ADDP: return fastEmit_AArch64ISD_ADDP_rr(VT, RetVT, Op0, Op1);
  case AArch64ISD::BIC: return fastEmit_AArch64ISD_BIC_rr(VT, RetVT, Op0, Op1);
  case AArch64ISD::CMEQ: return fastEmit_AArch64ISD_CMEQ_rr(VT, RetVT, Op0, Op1);
  case AArch64ISD::CMGE: return fastEmit_AArch64ISD_CMGE_rr(VT, RetVT, Op0, Op1);
  case AArch64ISD::CMGT: return fastEmit_AArch64ISD_CMGT_rr(VT, RetVT, Op0, Op1);
  case AArch64ISD::CMHI: return fastEmit_AArch64ISD_CMHI_rr(VT, RetVT, Op0, Op1);
  case AArch64ISD::CMHS: return fastEmit_AArch64ISD_CMHS_rr(VT, RetVT, Op0, Op1);
  case AArch64ISD::FCMEQ: return fastEmit_AArch64ISD_FCMEQ_rr(VT, RetVT, Op0, Op1);
  case AArch64ISD::FCMGE: return fastEmit_AArch64ISD_FCMGE_rr(VT, RetVT, Op0, Op1);
  case AArch64ISD::FCMGT: return fastEmit_AArch64ISD_FCMGT_rr(VT, RetVT, Op0, Op1);
  case AArch64ISD::FCMP: return fastEmit_AArch64ISD_FCMP_rr(VT, RetVT, Op0, Op1);
  case AArch64ISD::FRECPS: return fastEmit_AArch64ISD_FRECPS_rr(VT, RetVT, Op0, Op1);
  case AArch64ISD::FRSQRTS: return fastEmit_AArch64ISD_FRSQRTS_rr(VT, RetVT, Op0, Op1);
  case AArch64ISD::PMULL: return fastEmit_AArch64ISD_PMULL_rr(VT, RetVT, Op0, Op1);
  case AArch64ISD::PTEST: return fastEmit_AArch64ISD_PTEST_rr(VT, RetVT, Op0, Op1);
  case AArch64ISD::PTEST_ANY: return fastEmit_AArch64ISD_PTEST_ANY_rr(VT, RetVT, Op0, Op1);
  case AArch64ISD::SMULL: return fastEmit_AArch64ISD_SMULL_rr(VT, RetVT, Op0, Op1);
  case AArch64ISD::STRICT_FCMP: return fastEmit_AArch64ISD_STRICT_FCMP_rr(VT, RetVT, Op0, Op1);
  case AArch64ISD::STRICT_FCMPE: return fastEmit_AArch64ISD_STRICT_FCMPE_rr(VT, RetVT, Op0, Op1);
  case AArch64ISD::TBL: return fastEmit_AArch64ISD_TBL_rr(VT, RetVT, Op0, Op1);
  case AArch64ISD::TRN1: return fastEmit_AArch64ISD_TRN1_rr(VT, RetVT, Op0, Op1);
  case AArch64ISD::TRN2: return fastEmit_AArch64ISD_TRN2_rr(VT, RetVT, Op0, Op1);
  case AArch64ISD::UMULL: return fastEmit_AArch64ISD_UMULL_rr(VT, RetVT, Op0, Op1);
  case AArch64ISD::UZP1: return fastEmit_AArch64ISD_UZP1_rr(VT, RetVT, Op0, Op1);
  case AArch64ISD::UZP2: return fastEmit_AArch64ISD_UZP2_rr(VT, RetVT, Op0, Op1);
  case AArch64ISD::ZIP1: return fastEmit_AArch64ISD_ZIP1_rr(VT, RetVT, Op0, Op1);
  case AArch64ISD::ZIP2: return fastEmit_AArch64ISD_ZIP2_rr(VT, RetVT, Op0, Op1);
  case ISD::ABDS: return fastEmit_ISD_ABDS_rr(VT, RetVT, Op0, Op1);
  case ISD::ABDU: return fastEmit_ISD_ABDU_rr(VT, RetVT, Op0, Op1);
  case ISD::ADD: return fastEmit_ISD_ADD_rr(VT, RetVT, Op0, Op1);
  case ISD::AND: return fastEmit_ISD_AND_rr(VT, RetVT, Op0, Op1);
  case ISD::AVGCEILS: return fastEmit_ISD_AVGCEILS_rr(VT, RetVT, Op0, Op1);
  case ISD::AVGCEILU: return fastEmit_ISD_AVGCEILU_rr(VT, RetVT, Op0, Op1);
  case ISD::AVGFLOORS: return fastEmit_ISD_AVGFLOORS_rr(VT, RetVT, Op0, Op1);
  case ISD::AVGFLOORU: return fastEmit_ISD_AVGFLOORU_rr(VT, RetVT, Op0, Op1);
  case ISD::CONCAT_VECTORS: return fastEmit_ISD_CONCAT_VECTORS_rr(VT, RetVT, Op0, Op1);
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
  case ISD::STRICT_FADD: return fastEmit_ISD_STRICT_FADD_rr(VT, RetVT, Op0, Op1);
  case ISD::STRICT_FDIV: return fastEmit_ISD_STRICT_FDIV_rr(VT, RetVT, Op0, Op1);
  case ISD::STRICT_FMAXIMUM: return fastEmit_ISD_STRICT_FMAXIMUM_rr(VT, RetVT, Op0, Op1);
  case ISD::STRICT_FMAXNUM: return fastEmit_ISD_STRICT_FMAXNUM_rr(VT, RetVT, Op0, Op1);
  case ISD::STRICT_FMINIMUM: return fastEmit_ISD_STRICT_FMINIMUM_rr(VT, RetVT, Op0, Op1);
  case ISD::STRICT_FMINNUM: return fastEmit_ISD_STRICT_FMINNUM_rr(VT, RetVT, Op0, Op1);
  case ISD::STRICT_FMUL: return fastEmit_ISD_STRICT_FMUL_rr(VT, RetVT, Op0, Op1);
  case ISD::STRICT_FSUB: return fastEmit_ISD_STRICT_FSUB_rr(VT, RetVT, Op0, Op1);
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

// FastEmit functions for AArch64ISD::DUPLANE64.

unsigned fastEmit_AArch64ISD_DUPLANE64_MVT_v2i64_ri_Predicate_VectorIndexD(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::DUPv2i64lane, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_DUPLANE64_MVT_v2f64_ri_Predicate_VectorIndexD(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  return fastEmitInst_ri(AArch64::DUPv2i64lane, &AArch64::FPR128RegClass, Op0, imm1);
}

unsigned fastEmit_AArch64ISD_DUPLANE64_ri_Predicate_VectorIndexD(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v2i64: return fastEmit_AArch64ISD_DUPLANE64_MVT_v2i64_ri_Predicate_VectorIndexD(RetVT, Op0, imm1);
  case MVT::v2f64: return fastEmit_AArch64ISD_DUPLANE64_MVT_v2f64_ri_Predicate_VectorIndexD(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::EXTRACT_VECTOR_ELT.

unsigned fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v2i64_ri_Predicate_VectorIndexD(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::UMOVvi64, &AArch64::GPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v2f64_ri_Predicate_VectorIndexD(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  return fastEmitInst_ri(AArch64::DUPi64, &AArch64::FPR64RegClass, Op0, imm1);
}

unsigned fastEmit_ISD_EXTRACT_VECTOR_ELT_ri_Predicate_VectorIndexD(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v2i64: return fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v2i64_ri_Predicate_VectorIndexD(RetVT, Op0, imm1);
  case MVT::v2f64: return fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v2f64_ri_Predicate_VectorIndexD(RetVT, Op0, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_VectorIndexD(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, uint64_t imm1) {
  switch (Opcode) {
  case AArch64ISD::DUPLANE64: return fastEmit_AArch64ISD_DUPLANE64_ri_Predicate_VectorIndexD(VT, RetVT, Op0, imm1);
  case ISD::EXTRACT_VECTOR_ELT: return fastEmit_ISD_EXTRACT_VECTOR_ELT_ri_Predicate_VectorIndexD(VT, RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::DUPLANE32.

unsigned fastEmit_AArch64ISD_DUPLANE32_MVT_v4i32_MVT_v2i32_ri_Predicate_VectorIndexS(unsigned Op0, uint64_t imm1) {
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::DUPv2i32lane, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_DUPLANE32_MVT_v4i32_MVT_v4i32_ri_Predicate_VectorIndexS(unsigned Op0, uint64_t imm1) {
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::DUPv4i32lane, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_DUPLANE32_MVT_v4i32_ri_Predicate_VectorIndexS(MVT RetVT, unsigned Op0, uint64_t imm1) {
switch (RetVT.SimpleTy) {
  case MVT::v2i32: return fastEmit_AArch64ISD_DUPLANE32_MVT_v4i32_MVT_v2i32_ri_Predicate_VectorIndexS(Op0, imm1);
  case MVT::v4i32: return fastEmit_AArch64ISD_DUPLANE32_MVT_v4i32_MVT_v4i32_ri_Predicate_VectorIndexS(Op0, imm1);
  default: return 0;
}
}

unsigned fastEmit_AArch64ISD_DUPLANE32_MVT_v4f32_MVT_v2f32_ri_Predicate_VectorIndexS(unsigned Op0, uint64_t imm1) {
  return fastEmitInst_ri(AArch64::DUPv2i32lane, &AArch64::FPR64RegClass, Op0, imm1);
}

unsigned fastEmit_AArch64ISD_DUPLANE32_MVT_v4f32_MVT_v4f32_ri_Predicate_VectorIndexS(unsigned Op0, uint64_t imm1) {
  return fastEmitInst_ri(AArch64::DUPv4i32lane, &AArch64::FPR128RegClass, Op0, imm1);
}

unsigned fastEmit_AArch64ISD_DUPLANE32_MVT_v4f32_ri_Predicate_VectorIndexS(MVT RetVT, unsigned Op0, uint64_t imm1) {
switch (RetVT.SimpleTy) {
  case MVT::v2f32: return fastEmit_AArch64ISD_DUPLANE32_MVT_v4f32_MVT_v2f32_ri_Predicate_VectorIndexS(Op0, imm1);
  case MVT::v4f32: return fastEmit_AArch64ISD_DUPLANE32_MVT_v4f32_MVT_v4f32_ri_Predicate_VectorIndexS(Op0, imm1);
  default: return 0;
}
}

unsigned fastEmit_AArch64ISD_DUPLANE32_ri_Predicate_VectorIndexS(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v4i32: return fastEmit_AArch64ISD_DUPLANE32_MVT_v4i32_ri_Predicate_VectorIndexS(RetVT, Op0, imm1);
  case MVT::v4f32: return fastEmit_AArch64ISD_DUPLANE32_MVT_v4f32_ri_Predicate_VectorIndexS(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::EXTRACT_VECTOR_ELT.

unsigned fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v4i32_ri_Predicate_VectorIndexS(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::UMOVvi32, &AArch64::GPR32RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v4f32_ri_Predicate_VectorIndexS(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  return fastEmitInst_ri(AArch64::DUPi32, &AArch64::FPR32RegClass, Op0, imm1);
}

unsigned fastEmit_ISD_EXTRACT_VECTOR_ELT_ri_Predicate_VectorIndexS(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v4i32: return fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v4i32_ri_Predicate_VectorIndexS(RetVT, Op0, imm1);
  case MVT::v4f32: return fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v4f32_ri_Predicate_VectorIndexS(RetVT, Op0, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_VectorIndexS(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, uint64_t imm1) {
  switch (Opcode) {
  case AArch64ISD::DUPLANE32: return fastEmit_AArch64ISD_DUPLANE32_ri_Predicate_VectorIndexS(VT, RetVT, Op0, imm1);
  case ISD::EXTRACT_VECTOR_ELT: return fastEmit_ISD_EXTRACT_VECTOR_ELT_ri_Predicate_VectorIndexS(VT, RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::DUPLANE16.

unsigned fastEmit_AArch64ISD_DUPLANE16_MVT_v8i16_MVT_v4i16_ri_Predicate_VectorIndexH(unsigned Op0, uint64_t imm1) {
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::DUPv4i16lane, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_DUPLANE16_MVT_v8i16_MVT_v8i16_ri_Predicate_VectorIndexH(unsigned Op0, uint64_t imm1) {
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::DUPv8i16lane, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_DUPLANE16_MVT_v8i16_ri_Predicate_VectorIndexH(MVT RetVT, unsigned Op0, uint64_t imm1) {
switch (RetVT.SimpleTy) {
  case MVT::v4i16: return fastEmit_AArch64ISD_DUPLANE16_MVT_v8i16_MVT_v4i16_ri_Predicate_VectorIndexH(Op0, imm1);
  case MVT::v8i16: return fastEmit_AArch64ISD_DUPLANE16_MVT_v8i16_MVT_v8i16_ri_Predicate_VectorIndexH(Op0, imm1);
  default: return 0;
}
}

unsigned fastEmit_AArch64ISD_DUPLANE16_MVT_v8f16_MVT_v4f16_ri_Predicate_VectorIndexH(unsigned Op0, uint64_t imm1) {
  return fastEmitInst_ri(AArch64::DUPv4i16lane, &AArch64::FPR64RegClass, Op0, imm1);
}

unsigned fastEmit_AArch64ISD_DUPLANE16_MVT_v8f16_MVT_v8f16_ri_Predicate_VectorIndexH(unsigned Op0, uint64_t imm1) {
  return fastEmitInst_ri(AArch64::DUPv8i16lane, &AArch64::FPR128RegClass, Op0, imm1);
}

unsigned fastEmit_AArch64ISD_DUPLANE16_MVT_v8f16_ri_Predicate_VectorIndexH(MVT RetVT, unsigned Op0, uint64_t imm1) {
switch (RetVT.SimpleTy) {
  case MVT::v4f16: return fastEmit_AArch64ISD_DUPLANE16_MVT_v8f16_MVT_v4f16_ri_Predicate_VectorIndexH(Op0, imm1);
  case MVT::v8f16: return fastEmit_AArch64ISD_DUPLANE16_MVT_v8f16_MVT_v8f16_ri_Predicate_VectorIndexH(Op0, imm1);
  default: return 0;
}
}

unsigned fastEmit_AArch64ISD_DUPLANE16_MVT_v8bf16_MVT_v4bf16_ri_Predicate_VectorIndexH(unsigned Op0, uint64_t imm1) {
  return fastEmitInst_ri(AArch64::DUPv4i16lane, &AArch64::FPR64RegClass, Op0, imm1);
}

unsigned fastEmit_AArch64ISD_DUPLANE16_MVT_v8bf16_MVT_v8bf16_ri_Predicate_VectorIndexH(unsigned Op0, uint64_t imm1) {
  return fastEmitInst_ri(AArch64::DUPv8i16lane, &AArch64::FPR128RegClass, Op0, imm1);
}

unsigned fastEmit_AArch64ISD_DUPLANE16_MVT_v8bf16_ri_Predicate_VectorIndexH(MVT RetVT, unsigned Op0, uint64_t imm1) {
switch (RetVT.SimpleTy) {
  case MVT::v4bf16: return fastEmit_AArch64ISD_DUPLANE16_MVT_v8bf16_MVT_v4bf16_ri_Predicate_VectorIndexH(Op0, imm1);
  case MVT::v8bf16: return fastEmit_AArch64ISD_DUPLANE16_MVT_v8bf16_MVT_v8bf16_ri_Predicate_VectorIndexH(Op0, imm1);
  default: return 0;
}
}

unsigned fastEmit_AArch64ISD_DUPLANE16_ri_Predicate_VectorIndexH(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_AArch64ISD_DUPLANE16_MVT_v8i16_ri_Predicate_VectorIndexH(RetVT, Op0, imm1);
  case MVT::v8f16: return fastEmit_AArch64ISD_DUPLANE16_MVT_v8f16_ri_Predicate_VectorIndexH(RetVT, Op0, imm1);
  case MVT::v8bf16: return fastEmit_AArch64ISD_DUPLANE16_MVT_v8bf16_ri_Predicate_VectorIndexH(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::EXTRACT_VECTOR_ELT.

unsigned fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v8i16_ri_Predicate_VectorIndexH(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::UMOVvi16, &AArch64::GPR32RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v8f16_ri_Predicate_VectorIndexH(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  return fastEmitInst_ri(AArch64::DUPi16, &AArch64::FPR16RegClass, Op0, imm1);
}

unsigned fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v8bf16_ri_Predicate_VectorIndexH(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::bf16)
    return 0;
  return fastEmitInst_ri(AArch64::DUPi16, &AArch64::FPR16RegClass, Op0, imm1);
}

unsigned fastEmit_ISD_EXTRACT_VECTOR_ELT_ri_Predicate_VectorIndexH(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v8i16_ri_Predicate_VectorIndexH(RetVT, Op0, imm1);
  case MVT::v8f16: return fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v8f16_ri_Predicate_VectorIndexH(RetVT, Op0, imm1);
  case MVT::v8bf16: return fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v8bf16_ri_Predicate_VectorIndexH(RetVT, Op0, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_VectorIndexH(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, uint64_t imm1) {
  switch (Opcode) {
  case AArch64ISD::DUPLANE16: return fastEmit_AArch64ISD_DUPLANE16_ri_Predicate_VectorIndexH(VT, RetVT, Op0, imm1);
  case ISD::EXTRACT_VECTOR_ELT: return fastEmit_ISD_EXTRACT_VECTOR_ELT_ri_Predicate_VectorIndexH(VT, RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::DUPLANE8.

unsigned fastEmit_AArch64ISD_DUPLANE8_MVT_v16i8_MVT_v8i8_ri_Predicate_VectorIndexB(unsigned Op0, uint64_t imm1) {
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::DUPv8i8lane, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_DUPLANE8_MVT_v16i8_MVT_v16i8_ri_Predicate_VectorIndexB(unsigned Op0, uint64_t imm1) {
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::DUPv16i8lane, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_DUPLANE8_MVT_v16i8_ri_Predicate_VectorIndexB(MVT RetVT, unsigned Op0, uint64_t imm1) {
switch (RetVT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_DUPLANE8_MVT_v16i8_MVT_v8i8_ri_Predicate_VectorIndexB(Op0, imm1);
  case MVT::v16i8: return fastEmit_AArch64ISD_DUPLANE8_MVT_v16i8_MVT_v16i8_ri_Predicate_VectorIndexB(Op0, imm1);
  default: return 0;
}
}

unsigned fastEmit_AArch64ISD_DUPLANE8_ri_Predicate_VectorIndexB(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_AArch64ISD_DUPLANE8_MVT_v16i8_ri_Predicate_VectorIndexB(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::EXTRACT_VECTOR_ELT.

unsigned fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v16i8_ri_Predicate_VectorIndexB(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::UMOVvi8, &AArch64::GPR32RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_EXTRACT_VECTOR_ELT_ri_Predicate_VectorIndexB(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v16i8_ri_Predicate_VectorIndexB(RetVT, Op0, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_VectorIndexB(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, uint64_t imm1) {
  switch (Opcode) {
  case AArch64ISD::DUPLANE8: return fastEmit_AArch64ISD_DUPLANE8_ri_Predicate_VectorIndexB(VT, RetVT, Op0, imm1);
  case ISD::EXTRACT_VECTOR_ELT: return fastEmit_ISD_EXTRACT_VECTOR_ELT_ri_Predicate_VectorIndexB(VT, RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::EXTRACT_VECTOR_ELT.

unsigned fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v16i8_ri_Predicate_VectorIndex0(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasNEON() || Subtarget->hasSME())) {
    return fastEmitInst_ri(AArch64::UMOVvi8_idx0, &AArch64::GPR32RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v8i16_ri_Predicate_VectorIndex0(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasNEON() || Subtarget->hasSME())) {
    return fastEmitInst_ri(AArch64::UMOVvi16_idx0, &AArch64::GPR32RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v4i32_ri_Predicate_VectorIndex0(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasNEON() || Subtarget->hasSME())) {
    return fastEmitInst_ri(AArch64::UMOVvi32_idx0, &AArch64::GPR32RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v2i64_ri_Predicate_VectorIndex0(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasNEON() || Subtarget->hasSME())) {
    return fastEmitInst_ri(AArch64::UMOVvi64_idx0, &AArch64::GPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_EXTRACT_VECTOR_ELT_ri_Predicate_VectorIndex0(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v16i8_ri_Predicate_VectorIndex0(RetVT, Op0, imm1);
  case MVT::v8i16: return fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v8i16_ri_Predicate_VectorIndex0(RetVT, Op0, imm1);
  case MVT::v4i32: return fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v4i32_ri_Predicate_VectorIndex0(RetVT, Op0, imm1);
  case MVT::v2i64: return fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v2i64_ri_Predicate_VectorIndex0(RetVT, Op0, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_VectorIndex0(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, uint64_t imm1) {
  switch (Opcode) {
  case ISD::EXTRACT_VECTOR_ELT: return fastEmit_ISD_EXTRACT_VECTOR_ELT_ri_Predicate_VectorIndex0(VT, RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::SQSHLU_I.

unsigned fastEmit_AArch64ISD_SQSHLU_I_MVT_i64_ri_Predicate_vecshiftL64(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SQSHLUd, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SQSHLU_I_MVT_v1i64_ri_Predicate_vecshiftL64(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SQSHLUd, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SQSHLU_I_MVT_v2i64_ri_Predicate_vecshiftL64(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SQSHLUv2i64_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SQSHLU_I_ri_Predicate_vecshiftL64(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_AArch64ISD_SQSHLU_I_MVT_i64_ri_Predicate_vecshiftL64(RetVT, Op0, imm1);
  case MVT::v1i64: return fastEmit_AArch64ISD_SQSHLU_I_MVT_v1i64_ri_Predicate_vecshiftL64(RetVT, Op0, imm1);
  case MVT::v2i64: return fastEmit_AArch64ISD_SQSHLU_I_MVT_v2i64_ri_Predicate_vecshiftL64(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::SQSHL_I.

unsigned fastEmit_AArch64ISD_SQSHL_I_MVT_i64_ri_Predicate_vecshiftL64(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SQSHLd, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SQSHL_I_MVT_v1i64_ri_Predicate_vecshiftL64(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SQSHLd, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SQSHL_I_MVT_v2i64_ri_Predicate_vecshiftL64(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SQSHLv2i64_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SQSHL_I_ri_Predicate_vecshiftL64(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_AArch64ISD_SQSHL_I_MVT_i64_ri_Predicate_vecshiftL64(RetVT, Op0, imm1);
  case MVT::v1i64: return fastEmit_AArch64ISD_SQSHL_I_MVT_v1i64_ri_Predicate_vecshiftL64(RetVT, Op0, imm1);
  case MVT::v2i64: return fastEmit_AArch64ISD_SQSHL_I_MVT_v2i64_ri_Predicate_vecshiftL64(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::UQSHL_I.

unsigned fastEmit_AArch64ISD_UQSHL_I_MVT_i64_ri_Predicate_vecshiftL64(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::UQSHLd, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UQSHL_I_MVT_v1i64_ri_Predicate_vecshiftL64(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::UQSHLd, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UQSHL_I_MVT_v2i64_ri_Predicate_vecshiftL64(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::UQSHLv2i64_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UQSHL_I_ri_Predicate_vecshiftL64(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_AArch64ISD_UQSHL_I_MVT_i64_ri_Predicate_vecshiftL64(RetVT, Op0, imm1);
  case MVT::v1i64: return fastEmit_AArch64ISD_UQSHL_I_MVT_v1i64_ri_Predicate_vecshiftL64(RetVT, Op0, imm1);
  case MVT::v2i64: return fastEmit_AArch64ISD_UQSHL_I_MVT_v2i64_ri_Predicate_vecshiftL64(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::VSHL.

unsigned fastEmit_AArch64ISD_VSHL_MVT_i64_ri_Predicate_vecshiftL64(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SHLd, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_VSHL_MVT_v1i64_ri_Predicate_vecshiftL64(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SHLd, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_VSHL_MVT_v2i64_ri_Predicate_vecshiftL64(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SHLv2i64_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_VSHL_ri_Predicate_vecshiftL64(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_AArch64ISD_VSHL_MVT_i64_ri_Predicate_vecshiftL64(RetVT, Op0, imm1);
  case MVT::v1i64: return fastEmit_AArch64ISD_VSHL_MVT_v1i64_ri_Predicate_vecshiftL64(RetVT, Op0, imm1);
  case MVT::v2i64: return fastEmit_AArch64ISD_VSHL_MVT_v2i64_ri_Predicate_vecshiftL64(RetVT, Op0, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_vecshiftL64(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, uint64_t imm1) {
  switch (Opcode) {
  case AArch64ISD::SQSHLU_I: return fastEmit_AArch64ISD_SQSHLU_I_ri_Predicate_vecshiftL64(VT, RetVT, Op0, imm1);
  case AArch64ISD::SQSHL_I: return fastEmit_AArch64ISD_SQSHL_I_ri_Predicate_vecshiftL64(VT, RetVT, Op0, imm1);
  case AArch64ISD::UQSHL_I: return fastEmit_AArch64ISD_UQSHL_I_ri_Predicate_vecshiftL64(VT, RetVT, Op0, imm1);
  case AArch64ISD::VSHL: return fastEmit_AArch64ISD_VSHL_ri_Predicate_vecshiftL64(VT, RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::SQSHLU_I.

unsigned fastEmit_AArch64ISD_SQSHLU_I_MVT_i32_ri_Predicate_vecshiftL32(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SQSHLUs, &AArch64::FPR32RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SQSHLU_I_MVT_v2i32_ri_Predicate_vecshiftL32(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SQSHLUv2i32_shift, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SQSHLU_I_MVT_v4i32_ri_Predicate_vecshiftL32(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SQSHLUv4i32_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SQSHLU_I_ri_Predicate_vecshiftL32(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_AArch64ISD_SQSHLU_I_MVT_i32_ri_Predicate_vecshiftL32(RetVT, Op0, imm1);
  case MVT::v2i32: return fastEmit_AArch64ISD_SQSHLU_I_MVT_v2i32_ri_Predicate_vecshiftL32(RetVT, Op0, imm1);
  case MVT::v4i32: return fastEmit_AArch64ISD_SQSHLU_I_MVT_v4i32_ri_Predicate_vecshiftL32(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::SQSHL_I.

unsigned fastEmit_AArch64ISD_SQSHL_I_MVT_i32_ri_Predicate_vecshiftL32(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SQSHLs, &AArch64::FPR32RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SQSHL_I_MVT_v2i32_ri_Predicate_vecshiftL32(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SQSHLv2i32_shift, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SQSHL_I_MVT_v4i32_ri_Predicate_vecshiftL32(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SQSHLv4i32_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SQSHL_I_ri_Predicate_vecshiftL32(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_AArch64ISD_SQSHL_I_MVT_i32_ri_Predicate_vecshiftL32(RetVT, Op0, imm1);
  case MVT::v2i32: return fastEmit_AArch64ISD_SQSHL_I_MVT_v2i32_ri_Predicate_vecshiftL32(RetVT, Op0, imm1);
  case MVT::v4i32: return fastEmit_AArch64ISD_SQSHL_I_MVT_v4i32_ri_Predicate_vecshiftL32(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::UQSHL_I.

unsigned fastEmit_AArch64ISD_UQSHL_I_MVT_i32_ri_Predicate_vecshiftL32(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::UQSHLs, &AArch64::FPR32RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UQSHL_I_MVT_v2i32_ri_Predicate_vecshiftL32(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::UQSHLv2i32_shift, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UQSHL_I_MVT_v4i32_ri_Predicate_vecshiftL32(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::UQSHLv4i32_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UQSHL_I_ri_Predicate_vecshiftL32(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_AArch64ISD_UQSHL_I_MVT_i32_ri_Predicate_vecshiftL32(RetVT, Op0, imm1);
  case MVT::v2i32: return fastEmit_AArch64ISD_UQSHL_I_MVT_v2i32_ri_Predicate_vecshiftL32(RetVT, Op0, imm1);
  case MVT::v4i32: return fastEmit_AArch64ISD_UQSHL_I_MVT_v4i32_ri_Predicate_vecshiftL32(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::VSHL.

unsigned fastEmit_AArch64ISD_VSHL_MVT_v2i32_ri_Predicate_vecshiftL32(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SHLv2i32_shift, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_VSHL_MVT_v4i32_ri_Predicate_vecshiftL32(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SHLv4i32_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_VSHL_ri_Predicate_vecshiftL32(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v2i32: return fastEmit_AArch64ISD_VSHL_MVT_v2i32_ri_Predicate_vecshiftL32(RetVT, Op0, imm1);
  case MVT::v4i32: return fastEmit_AArch64ISD_VSHL_MVT_v4i32_ri_Predicate_vecshiftL32(RetVT, Op0, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_vecshiftL32(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, uint64_t imm1) {
  switch (Opcode) {
  case AArch64ISD::SQSHLU_I: return fastEmit_AArch64ISD_SQSHLU_I_ri_Predicate_vecshiftL32(VT, RetVT, Op0, imm1);
  case AArch64ISD::SQSHL_I: return fastEmit_AArch64ISD_SQSHL_I_ri_Predicate_vecshiftL32(VT, RetVT, Op0, imm1);
  case AArch64ISD::UQSHL_I: return fastEmit_AArch64ISD_UQSHL_I_ri_Predicate_vecshiftL32(VT, RetVT, Op0, imm1);
  case AArch64ISD::VSHL: return fastEmit_AArch64ISD_VSHL_ri_Predicate_vecshiftL32(VT, RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::SRSHR_I.

unsigned fastEmit_AArch64ISD_SRSHR_I_MVT_i64_ri_Predicate_vecshiftR64(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SRSHRd, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SRSHR_I_MVT_v1i64_ri_Predicate_vecshiftR64(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SRSHRd, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SRSHR_I_MVT_v2i64_ri_Predicate_vecshiftR64(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SRSHRv2i64_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SRSHR_I_ri_Predicate_vecshiftR64(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_AArch64ISD_SRSHR_I_MVT_i64_ri_Predicate_vecshiftR64(RetVT, Op0, imm1);
  case MVT::v1i64: return fastEmit_AArch64ISD_SRSHR_I_MVT_v1i64_ri_Predicate_vecshiftR64(RetVT, Op0, imm1);
  case MVT::v2i64: return fastEmit_AArch64ISD_SRSHR_I_MVT_v2i64_ri_Predicate_vecshiftR64(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::URSHR_I.

unsigned fastEmit_AArch64ISD_URSHR_I_MVT_i64_ri_Predicate_vecshiftR64(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::URSHRd, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_URSHR_I_MVT_v1i64_ri_Predicate_vecshiftR64(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::URSHRd, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_URSHR_I_MVT_v2i64_ri_Predicate_vecshiftR64(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::URSHRv2i64_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_URSHR_I_ri_Predicate_vecshiftR64(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_AArch64ISD_URSHR_I_MVT_i64_ri_Predicate_vecshiftR64(RetVT, Op0, imm1);
  case MVT::v1i64: return fastEmit_AArch64ISD_URSHR_I_MVT_v1i64_ri_Predicate_vecshiftR64(RetVT, Op0, imm1);
  case MVT::v2i64: return fastEmit_AArch64ISD_URSHR_I_MVT_v2i64_ri_Predicate_vecshiftR64(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::VASHR.

unsigned fastEmit_AArch64ISD_VASHR_MVT_i64_ri_Predicate_vecshiftR64(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SSHRd, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_VASHR_MVT_v1i64_ri_Predicate_vecshiftR64(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SSHRd, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_VASHR_MVT_v2i64_ri_Predicate_vecshiftR64(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SSHRv2i64_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_VASHR_ri_Predicate_vecshiftR64(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_AArch64ISD_VASHR_MVT_i64_ri_Predicate_vecshiftR64(RetVT, Op0, imm1);
  case MVT::v1i64: return fastEmit_AArch64ISD_VASHR_MVT_v1i64_ri_Predicate_vecshiftR64(RetVT, Op0, imm1);
  case MVT::v2i64: return fastEmit_AArch64ISD_VASHR_MVT_v2i64_ri_Predicate_vecshiftR64(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::VLSHR.

unsigned fastEmit_AArch64ISD_VLSHR_MVT_i64_ri_Predicate_vecshiftR64(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::USHRd, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_VLSHR_MVT_v1i64_ri_Predicate_vecshiftR64(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v1i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::USHRd, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_VLSHR_MVT_v2i64_ri_Predicate_vecshiftR64(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::USHRv2i64_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_VLSHR_ri_Predicate_vecshiftR64(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_AArch64ISD_VLSHR_MVT_i64_ri_Predicate_vecshiftR64(RetVT, Op0, imm1);
  case MVT::v1i64: return fastEmit_AArch64ISD_VLSHR_MVT_v1i64_ri_Predicate_vecshiftR64(RetVT, Op0, imm1);
  case MVT::v2i64: return fastEmit_AArch64ISD_VLSHR_MVT_v2i64_ri_Predicate_vecshiftR64(RetVT, Op0, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_vecshiftR64(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, uint64_t imm1) {
  switch (Opcode) {
  case AArch64ISD::SRSHR_I: return fastEmit_AArch64ISD_SRSHR_I_ri_Predicate_vecshiftR64(VT, RetVT, Op0, imm1);
  case AArch64ISD::URSHR_I: return fastEmit_AArch64ISD_URSHR_I_ri_Predicate_vecshiftR64(VT, RetVT, Op0, imm1);
  case AArch64ISD::VASHR: return fastEmit_AArch64ISD_VASHR_ri_Predicate_vecshiftR64(VT, RetVT, Op0, imm1);
  case AArch64ISD::VLSHR: return fastEmit_AArch64ISD_VLSHR_ri_Predicate_vecshiftR64(VT, RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::SQSHLU_I.

unsigned fastEmit_AArch64ISD_SQSHLU_I_MVT_v8i8_ri_Predicate_vecshiftL8(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SQSHLUv8i8_shift, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SQSHLU_I_MVT_v16i8_ri_Predicate_vecshiftL8(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SQSHLUv16i8_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SQSHLU_I_ri_Predicate_vecshiftL8(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_SQSHLU_I_MVT_v8i8_ri_Predicate_vecshiftL8(RetVT, Op0, imm1);
  case MVT::v16i8: return fastEmit_AArch64ISD_SQSHLU_I_MVT_v16i8_ri_Predicate_vecshiftL8(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::SQSHL_I.

unsigned fastEmit_AArch64ISD_SQSHL_I_MVT_v8i8_ri_Predicate_vecshiftL8(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SQSHLv8i8_shift, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SQSHL_I_MVT_v16i8_ri_Predicate_vecshiftL8(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SQSHLv16i8_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SQSHL_I_ri_Predicate_vecshiftL8(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_SQSHL_I_MVT_v8i8_ri_Predicate_vecshiftL8(RetVT, Op0, imm1);
  case MVT::v16i8: return fastEmit_AArch64ISD_SQSHL_I_MVT_v16i8_ri_Predicate_vecshiftL8(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::UQSHL_I.

unsigned fastEmit_AArch64ISD_UQSHL_I_MVT_v8i8_ri_Predicate_vecshiftL8(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::UQSHLv8i8_shift, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UQSHL_I_MVT_v16i8_ri_Predicate_vecshiftL8(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::UQSHLv16i8_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UQSHL_I_ri_Predicate_vecshiftL8(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_UQSHL_I_MVT_v8i8_ri_Predicate_vecshiftL8(RetVT, Op0, imm1);
  case MVT::v16i8: return fastEmit_AArch64ISD_UQSHL_I_MVT_v16i8_ri_Predicate_vecshiftL8(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::VSHL.

unsigned fastEmit_AArch64ISD_VSHL_MVT_v8i8_ri_Predicate_vecshiftL8(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SHLv8i8_shift, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_VSHL_MVT_v16i8_ri_Predicate_vecshiftL8(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SHLv16i8_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_VSHL_ri_Predicate_vecshiftL8(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_VSHL_MVT_v8i8_ri_Predicate_vecshiftL8(RetVT, Op0, imm1);
  case MVT::v16i8: return fastEmit_AArch64ISD_VSHL_MVT_v16i8_ri_Predicate_vecshiftL8(RetVT, Op0, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_vecshiftL8(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, uint64_t imm1) {
  switch (Opcode) {
  case AArch64ISD::SQSHLU_I: return fastEmit_AArch64ISD_SQSHLU_I_ri_Predicate_vecshiftL8(VT, RetVT, Op0, imm1);
  case AArch64ISD::SQSHL_I: return fastEmit_AArch64ISD_SQSHL_I_ri_Predicate_vecshiftL8(VT, RetVT, Op0, imm1);
  case AArch64ISD::UQSHL_I: return fastEmit_AArch64ISD_UQSHL_I_ri_Predicate_vecshiftL8(VT, RetVT, Op0, imm1);
  case AArch64ISD::VSHL: return fastEmit_AArch64ISD_VSHL_ri_Predicate_vecshiftL8(VT, RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::SQSHLU_I.

unsigned fastEmit_AArch64ISD_SQSHLU_I_MVT_v4i16_ri_Predicate_vecshiftL16(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SQSHLUv4i16_shift, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SQSHLU_I_MVT_v8i16_ri_Predicate_vecshiftL16(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SQSHLUv8i16_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SQSHLU_I_ri_Predicate_vecshiftL16(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v4i16: return fastEmit_AArch64ISD_SQSHLU_I_MVT_v4i16_ri_Predicate_vecshiftL16(RetVT, Op0, imm1);
  case MVT::v8i16: return fastEmit_AArch64ISD_SQSHLU_I_MVT_v8i16_ri_Predicate_vecshiftL16(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::SQSHL_I.

unsigned fastEmit_AArch64ISD_SQSHL_I_MVT_v4i16_ri_Predicate_vecshiftL16(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SQSHLv4i16_shift, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SQSHL_I_MVT_v8i16_ri_Predicate_vecshiftL16(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SQSHLv8i16_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SQSHL_I_ri_Predicate_vecshiftL16(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v4i16: return fastEmit_AArch64ISD_SQSHL_I_MVT_v4i16_ri_Predicate_vecshiftL16(RetVT, Op0, imm1);
  case MVT::v8i16: return fastEmit_AArch64ISD_SQSHL_I_MVT_v8i16_ri_Predicate_vecshiftL16(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::UQSHL_I.

unsigned fastEmit_AArch64ISD_UQSHL_I_MVT_v4i16_ri_Predicate_vecshiftL16(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::UQSHLv4i16_shift, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UQSHL_I_MVT_v8i16_ri_Predicate_vecshiftL16(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::UQSHLv8i16_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_UQSHL_I_ri_Predicate_vecshiftL16(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v4i16: return fastEmit_AArch64ISD_UQSHL_I_MVT_v4i16_ri_Predicate_vecshiftL16(RetVT, Op0, imm1);
  case MVT::v8i16: return fastEmit_AArch64ISD_UQSHL_I_MVT_v8i16_ri_Predicate_vecshiftL16(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::VSHL.

unsigned fastEmit_AArch64ISD_VSHL_MVT_v4i16_ri_Predicate_vecshiftL16(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SHLv4i16_shift, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_VSHL_MVT_v8i16_ri_Predicate_vecshiftL16(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SHLv8i16_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_VSHL_ri_Predicate_vecshiftL16(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v4i16: return fastEmit_AArch64ISD_VSHL_MVT_v4i16_ri_Predicate_vecshiftL16(RetVT, Op0, imm1);
  case MVT::v8i16: return fastEmit_AArch64ISD_VSHL_MVT_v8i16_ri_Predicate_vecshiftL16(RetVT, Op0, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_vecshiftL16(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, uint64_t imm1) {
  switch (Opcode) {
  case AArch64ISD::SQSHLU_I: return fastEmit_AArch64ISD_SQSHLU_I_ri_Predicate_vecshiftL16(VT, RetVT, Op0, imm1);
  case AArch64ISD::SQSHL_I: return fastEmit_AArch64ISD_SQSHL_I_ri_Predicate_vecshiftL16(VT, RetVT, Op0, imm1);
  case AArch64ISD::UQSHL_I: return fastEmit_AArch64ISD_UQSHL_I_ri_Predicate_vecshiftL16(VT, RetVT, Op0, imm1);
  case AArch64ISD::VSHL: return fastEmit_AArch64ISD_VSHL_ri_Predicate_vecshiftL16(VT, RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::SRSHR_I.

unsigned fastEmit_AArch64ISD_SRSHR_I_MVT_v8i8_ri_Predicate_vecshiftR8(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SRSHRv8i8_shift, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SRSHR_I_MVT_v16i8_ri_Predicate_vecshiftR8(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SRSHRv16i8_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SRSHR_I_ri_Predicate_vecshiftR8(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_SRSHR_I_MVT_v8i8_ri_Predicate_vecshiftR8(RetVT, Op0, imm1);
  case MVT::v16i8: return fastEmit_AArch64ISD_SRSHR_I_MVT_v16i8_ri_Predicate_vecshiftR8(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::URSHR_I.

unsigned fastEmit_AArch64ISD_URSHR_I_MVT_v8i8_ri_Predicate_vecshiftR8(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::URSHRv8i8_shift, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_URSHR_I_MVT_v16i8_ri_Predicate_vecshiftR8(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::URSHRv16i8_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_URSHR_I_ri_Predicate_vecshiftR8(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_URSHR_I_MVT_v8i8_ri_Predicate_vecshiftR8(RetVT, Op0, imm1);
  case MVT::v16i8: return fastEmit_AArch64ISD_URSHR_I_MVT_v16i8_ri_Predicate_vecshiftR8(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::VASHR.

unsigned fastEmit_AArch64ISD_VASHR_MVT_v8i8_ri_Predicate_vecshiftR8(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SSHRv8i8_shift, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_VASHR_MVT_v16i8_ri_Predicate_vecshiftR8(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SSHRv16i8_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_VASHR_ri_Predicate_vecshiftR8(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_VASHR_MVT_v8i8_ri_Predicate_vecshiftR8(RetVT, Op0, imm1);
  case MVT::v16i8: return fastEmit_AArch64ISD_VASHR_MVT_v16i8_ri_Predicate_vecshiftR8(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::VLSHR.

unsigned fastEmit_AArch64ISD_VLSHR_MVT_v8i8_ri_Predicate_vecshiftR8(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::USHRv8i8_shift, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_VLSHR_MVT_v16i8_ri_Predicate_vecshiftR8(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::USHRv16i8_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_VLSHR_ri_Predicate_vecshiftR8(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_VLSHR_MVT_v8i8_ri_Predicate_vecshiftR8(RetVT, Op0, imm1);
  case MVT::v16i8: return fastEmit_AArch64ISD_VLSHR_MVT_v16i8_ri_Predicate_vecshiftR8(RetVT, Op0, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_vecshiftR8(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, uint64_t imm1) {
  switch (Opcode) {
  case AArch64ISD::SRSHR_I: return fastEmit_AArch64ISD_SRSHR_I_ri_Predicate_vecshiftR8(VT, RetVT, Op0, imm1);
  case AArch64ISD::URSHR_I: return fastEmit_AArch64ISD_URSHR_I_ri_Predicate_vecshiftR8(VT, RetVT, Op0, imm1);
  case AArch64ISD::VASHR: return fastEmit_AArch64ISD_VASHR_ri_Predicate_vecshiftR8(VT, RetVT, Op0, imm1);
  case AArch64ISD::VLSHR: return fastEmit_AArch64ISD_VLSHR_ri_Predicate_vecshiftR8(VT, RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::SRSHR_I.

unsigned fastEmit_AArch64ISD_SRSHR_I_MVT_v4i16_ri_Predicate_vecshiftR16(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SRSHRv4i16_shift, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SRSHR_I_MVT_v8i16_ri_Predicate_vecshiftR16(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SRSHRv8i16_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SRSHR_I_ri_Predicate_vecshiftR16(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v4i16: return fastEmit_AArch64ISD_SRSHR_I_MVT_v4i16_ri_Predicate_vecshiftR16(RetVT, Op0, imm1);
  case MVT::v8i16: return fastEmit_AArch64ISD_SRSHR_I_MVT_v8i16_ri_Predicate_vecshiftR16(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::URSHR_I.

unsigned fastEmit_AArch64ISD_URSHR_I_MVT_v4i16_ri_Predicate_vecshiftR16(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::URSHRv4i16_shift, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_URSHR_I_MVT_v8i16_ri_Predicate_vecshiftR16(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::URSHRv8i16_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_URSHR_I_ri_Predicate_vecshiftR16(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v4i16: return fastEmit_AArch64ISD_URSHR_I_MVT_v4i16_ri_Predicate_vecshiftR16(RetVT, Op0, imm1);
  case MVT::v8i16: return fastEmit_AArch64ISD_URSHR_I_MVT_v8i16_ri_Predicate_vecshiftR16(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::VASHR.

unsigned fastEmit_AArch64ISD_VASHR_MVT_v4i16_ri_Predicate_vecshiftR16(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SSHRv4i16_shift, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_VASHR_MVT_v8i16_ri_Predicate_vecshiftR16(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SSHRv8i16_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_VASHR_ri_Predicate_vecshiftR16(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v4i16: return fastEmit_AArch64ISD_VASHR_MVT_v4i16_ri_Predicate_vecshiftR16(RetVT, Op0, imm1);
  case MVT::v8i16: return fastEmit_AArch64ISD_VASHR_MVT_v8i16_ri_Predicate_vecshiftR16(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::VLSHR.

unsigned fastEmit_AArch64ISD_VLSHR_MVT_v4i16_ri_Predicate_vecshiftR16(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::USHRv4i16_shift, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_VLSHR_MVT_v8i16_ri_Predicate_vecshiftR16(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::USHRv8i16_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_VLSHR_ri_Predicate_vecshiftR16(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v4i16: return fastEmit_AArch64ISD_VLSHR_MVT_v4i16_ri_Predicate_vecshiftR16(RetVT, Op0, imm1);
  case MVT::v8i16: return fastEmit_AArch64ISD_VLSHR_MVT_v8i16_ri_Predicate_vecshiftR16(RetVT, Op0, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_vecshiftR16(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, uint64_t imm1) {
  switch (Opcode) {
  case AArch64ISD::SRSHR_I: return fastEmit_AArch64ISD_SRSHR_I_ri_Predicate_vecshiftR16(VT, RetVT, Op0, imm1);
  case AArch64ISD::URSHR_I: return fastEmit_AArch64ISD_URSHR_I_ri_Predicate_vecshiftR16(VT, RetVT, Op0, imm1);
  case AArch64ISD::VASHR: return fastEmit_AArch64ISD_VASHR_ri_Predicate_vecshiftR16(VT, RetVT, Op0, imm1);
  case AArch64ISD::VLSHR: return fastEmit_AArch64ISD_VLSHR_ri_Predicate_vecshiftR16(VT, RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::SRSHR_I.

unsigned fastEmit_AArch64ISD_SRSHR_I_MVT_v2i32_ri_Predicate_vecshiftR32(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SRSHRv2i32_shift, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SRSHR_I_MVT_v4i32_ri_Predicate_vecshiftR32(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SRSHRv4i32_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_SRSHR_I_ri_Predicate_vecshiftR32(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v2i32: return fastEmit_AArch64ISD_SRSHR_I_MVT_v2i32_ri_Predicate_vecshiftR32(RetVT, Op0, imm1);
  case MVT::v4i32: return fastEmit_AArch64ISD_SRSHR_I_MVT_v4i32_ri_Predicate_vecshiftR32(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::URSHR_I.

unsigned fastEmit_AArch64ISD_URSHR_I_MVT_v2i32_ri_Predicate_vecshiftR32(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::URSHRv2i32_shift, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_URSHR_I_MVT_v4i32_ri_Predicate_vecshiftR32(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::URSHRv4i32_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_URSHR_I_ri_Predicate_vecshiftR32(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v2i32: return fastEmit_AArch64ISD_URSHR_I_MVT_v2i32_ri_Predicate_vecshiftR32(RetVT, Op0, imm1);
  case MVT::v4i32: return fastEmit_AArch64ISD_URSHR_I_MVT_v4i32_ri_Predicate_vecshiftR32(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::VASHR.

unsigned fastEmit_AArch64ISD_VASHR_MVT_v2i32_ri_Predicate_vecshiftR32(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SSHRv2i32_shift, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_VASHR_MVT_v4i32_ri_Predicate_vecshiftR32(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::SSHRv4i32_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_VASHR_ri_Predicate_vecshiftR32(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v2i32: return fastEmit_AArch64ISD_VASHR_MVT_v2i32_ri_Predicate_vecshiftR32(RetVT, Op0, imm1);
  case MVT::v4i32: return fastEmit_AArch64ISD_VASHR_MVT_v4i32_ri_Predicate_vecshiftR32(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::VLSHR.

unsigned fastEmit_AArch64ISD_VLSHR_MVT_v2i32_ri_Predicate_vecshiftR32(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::USHRv2i32_shift, &AArch64::FPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_VLSHR_MVT_v4i32_ri_Predicate_vecshiftR32(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_ri(AArch64::USHRv4i32_shift, &AArch64::FPR128RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_VLSHR_ri_Predicate_vecshiftR32(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v2i32: return fastEmit_AArch64ISD_VLSHR_MVT_v2i32_ri_Predicate_vecshiftR32(RetVT, Op0, imm1);
  case MVT::v4i32: return fastEmit_AArch64ISD_VLSHR_MVT_v4i32_ri_Predicate_vecshiftR32(RetVT, Op0, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_vecshiftR32(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, uint64_t imm1) {
  switch (Opcode) {
  case AArch64ISD::SRSHR_I: return fastEmit_AArch64ISD_SRSHR_I_ri_Predicate_vecshiftR32(VT, RetVT, Op0, imm1);
  case AArch64ISD::URSHR_I: return fastEmit_AArch64ISD_URSHR_I_ri_Predicate_vecshiftR32(VT, RetVT, Op0, imm1);
  case AArch64ISD::VASHR: return fastEmit_AArch64ISD_VASHR_ri_Predicate_vecshiftR32(VT, RetVT, Op0, imm1);
  case AArch64ISD::VLSHR: return fastEmit_AArch64ISD_VLSHR_ri_Predicate_vecshiftR32(VT, RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SMAX.

unsigned fastEmit_ISD_SMAX_MVT_i32_ri_Predicate_simm8_32b(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasCSSC())) {
    return fastEmitInst_ri(AArch64::SMAXWri, &AArch64::GPR32RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_ri_Predicate_simm8_32b(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SMAX_MVT_i32_ri_Predicate_simm8_32b(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SMIN.

unsigned fastEmit_ISD_SMIN_MVT_i32_ri_Predicate_simm8_32b(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasCSSC())) {
    return fastEmitInst_ri(AArch64::SMINWri, &AArch64::GPR32RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_ri_Predicate_simm8_32b(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SMIN_MVT_i32_ri_Predicate_simm8_32b(RetVT, Op0, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_simm8_32b(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, uint64_t imm1) {
  switch (Opcode) {
  case ISD::SMAX: return fastEmit_ISD_SMAX_ri_Predicate_simm8_32b(VT, RetVT, Op0, imm1);
  case ISD::SMIN: return fastEmit_ISD_SMIN_ri_Predicate_simm8_32b(VT, RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SMAX.

unsigned fastEmit_ISD_SMAX_MVT_i64_ri_Predicate_simm8_64b(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasCSSC())) {
    return fastEmitInst_ri(AArch64::SMAXXri, &AArch64::GPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_ri_Predicate_simm8_64b(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_SMAX_MVT_i64_ri_Predicate_simm8_64b(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SMIN.

unsigned fastEmit_ISD_SMIN_MVT_i64_ri_Predicate_simm8_64b(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasCSSC())) {
    return fastEmitInst_ri(AArch64::SMINXri, &AArch64::GPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_ri_Predicate_simm8_64b(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_SMIN_MVT_i64_ri_Predicate_simm8_64b(RetVT, Op0, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_simm8_64b(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, uint64_t imm1) {
  switch (Opcode) {
  case ISD::SMAX: return fastEmit_ISD_SMAX_ri_Predicate_simm8_64b(VT, RetVT, Op0, imm1);
  case ISD::SMIN: return fastEmit_ISD_SMIN_ri_Predicate_simm8_64b(VT, RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::UMAX.

unsigned fastEmit_ISD_UMAX_MVT_i32_ri_Predicate_uimm8_32b(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasCSSC())) {
    return fastEmitInst_ri(AArch64::UMAXWri, &AArch64::GPR32RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_ri_Predicate_uimm8_32b(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_UMAX_MVT_i32_ri_Predicate_uimm8_32b(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::UMIN.

unsigned fastEmit_ISD_UMIN_MVT_i32_ri_Predicate_uimm8_32b(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasCSSC())) {
    return fastEmitInst_ri(AArch64::UMINWri, &AArch64::GPR32RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_ri_Predicate_uimm8_32b(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_UMIN_MVT_i32_ri_Predicate_uimm8_32b(RetVT, Op0, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_uimm8_32b(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, uint64_t imm1) {
  switch (Opcode) {
  case ISD::UMAX: return fastEmit_ISD_UMAX_ri_Predicate_uimm8_32b(VT, RetVT, Op0, imm1);
  case ISD::UMIN: return fastEmit_ISD_UMIN_ri_Predicate_uimm8_32b(VT, RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::UMAX.

unsigned fastEmit_ISD_UMAX_MVT_i64_ri_Predicate_uimm8_64b(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasCSSC())) {
    return fastEmitInst_ri(AArch64::UMAXXri, &AArch64::GPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_ri_Predicate_uimm8_64b(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_UMAX_MVT_i64_ri_Predicate_uimm8_64b(RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::UMIN.

unsigned fastEmit_ISD_UMIN_MVT_i64_ri_Predicate_uimm8_64b(MVT RetVT, unsigned Op0, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasCSSC())) {
    return fastEmitInst_ri(AArch64::UMINXri, &AArch64::GPR64RegClass, Op0, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_ri_Predicate_uimm8_64b(MVT VT, MVT RetVT, unsigned Op0, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_UMIN_MVT_i64_ri_Predicate_uimm8_64b(RetVT, Op0, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_uimm8_64b(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, uint64_t imm1) {
  switch (Opcode) {
  case ISD::UMAX: return fastEmit_ISD_UMAX_ri_Predicate_uimm8_64b(VT, RetVT, Op0, imm1);
  case ISD::UMIN: return fastEmit_ISD_UMIN_ri_Predicate_uimm8_64b(VT, RetVT, Op0, imm1);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::MRS.

unsigned fastEmit_AArch64ISD_MRS_MVT_i32_i(MVT RetVT, uint64_t imm0) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_i(AArch64::MRS, &AArch64::GPR64RegClass, imm0);
}

unsigned fastEmit_AArch64ISD_MRS_i(MVT VT, MVT RetVT, uint64_t imm0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_AArch64ISD_MRS_MVT_i32_i(RetVT, imm0);
  default: return 0;
  }
}

// FastEmit functions for ISD::Constant.

unsigned fastEmit_ISD_Constant_MVT_i32_i(MVT RetVT, uint64_t imm0) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_i(AArch64::MOVi32imm, &AArch64::GPR32RegClass, imm0);
}

unsigned fastEmit_ISD_Constant_MVT_i64_i(MVT RetVT, uint64_t imm0) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_i(AArch64::MOVi64imm, &AArch64::GPR64RegClass, imm0);
}

unsigned fastEmit_ISD_Constant_i(MVT VT, MVT RetVT, uint64_t imm0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_Constant_MVT_i32_i(RetVT, imm0);
  case MVT::i64: return fastEmit_ISD_Constant_MVT_i64_i(RetVT, imm0);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_i(MVT VT, MVT RetVT, unsigned Opcode, uint64_t imm0) override {
  if (VT == MVT::i32 && Predicate_imm0_255(imm0))
    if (unsigned Reg = fastEmit_i_Predicate_imm0_255(VT, RetVT, Opcode, imm0))
      return Reg;

  if (VT == MVT::i32 && Predicate_simm6_32b(imm0))
    if (unsigned Reg = fastEmit_i_Predicate_simm6_32b(VT, RetVT, Opcode, imm0))
      return Reg;

  switch (Opcode) {
  case AArch64ISD::MRS: return fastEmit_AArch64ISD_MRS_i(VT, RetVT, imm0);
  case ISD::Constant: return fastEmit_ISD_Constant_i(VT, RetVT, imm0);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::FMOV.

unsigned fastEmit_AArch64ISD_FMOV_MVT_i32_MVT_v4f16_i_Predicate_imm0_255(uint64_t imm0) {
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_i(AArch64::FMOVv4f16_ns, &AArch64::FPR64RegClass, imm0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FMOV_MVT_i32_MVT_v8f16_i_Predicate_imm0_255(uint64_t imm0) {
  if ((Subtarget->hasFullFP16()) && (Subtarget->hasNEON())) {
    return fastEmitInst_i(AArch64::FMOVv8f16_ns, &AArch64::FPR128RegClass, imm0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FMOV_MVT_i32_MVT_v2f32_i_Predicate_imm0_255(uint64_t imm0) {
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_i(AArch64::FMOVv2f32_ns, &AArch64::FPR64RegClass, imm0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FMOV_MVT_i32_MVT_v4f32_i_Predicate_imm0_255(uint64_t imm0) {
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_i(AArch64::FMOVv4f32_ns, &AArch64::FPR128RegClass, imm0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FMOV_MVT_i32_MVT_v2f64_i_Predicate_imm0_255(uint64_t imm0) {
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_i(AArch64::FMOVv2f64_ns, &AArch64::FPR128RegClass, imm0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_FMOV_MVT_i32_i_Predicate_imm0_255(MVT RetVT, uint64_t imm0) {
switch (RetVT.SimpleTy) {
  case MVT::v4f16: return fastEmit_AArch64ISD_FMOV_MVT_i32_MVT_v4f16_i_Predicate_imm0_255(imm0);
  case MVT::v8f16: return fastEmit_AArch64ISD_FMOV_MVT_i32_MVT_v8f16_i_Predicate_imm0_255(imm0);
  case MVT::v2f32: return fastEmit_AArch64ISD_FMOV_MVT_i32_MVT_v2f32_i_Predicate_imm0_255(imm0);
  case MVT::v4f32: return fastEmit_AArch64ISD_FMOV_MVT_i32_MVT_v4f32_i_Predicate_imm0_255(imm0);
  case MVT::v2f64: return fastEmit_AArch64ISD_FMOV_MVT_i32_MVT_v2f64_i_Predicate_imm0_255(imm0);
  default: return 0;
}
}

unsigned fastEmit_AArch64ISD_FMOV_i_Predicate_imm0_255(MVT VT, MVT RetVT, uint64_t imm0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_AArch64ISD_FMOV_MVT_i32_i_Predicate_imm0_255(RetVT, imm0);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::MOVI.

unsigned fastEmit_AArch64ISD_MOVI_MVT_i32_MVT_v8i8_i_Predicate_imm0_255(uint64_t imm0) {
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_i(AArch64::MOVIv8b_ns, &AArch64::FPR64RegClass, imm0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_MOVI_MVT_i32_MVT_v16i8_i_Predicate_imm0_255(uint64_t imm0) {
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_i(AArch64::MOVIv16b_ns, &AArch64::FPR128RegClass, imm0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_MOVI_MVT_i32_i_Predicate_imm0_255(MVT RetVT, uint64_t imm0) {
switch (RetVT.SimpleTy) {
  case MVT::v8i8: return fastEmit_AArch64ISD_MOVI_MVT_i32_MVT_v8i8_i_Predicate_imm0_255(imm0);
  case MVT::v16i8: return fastEmit_AArch64ISD_MOVI_MVT_i32_MVT_v16i8_i_Predicate_imm0_255(imm0);
  default: return 0;
}
}

unsigned fastEmit_AArch64ISD_MOVI_i_Predicate_imm0_255(MVT VT, MVT RetVT, uint64_t imm0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_AArch64ISD_MOVI_MVT_i32_i_Predicate_imm0_255(RetVT, imm0);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::MOVIedit.

unsigned fastEmit_AArch64ISD_MOVIedit_MVT_i32_MVT_f64_i_Predicate_imm0_255(uint64_t imm0) {
  return fastEmitInst_i(AArch64::MOVID, &AArch64::FPR64RegClass, imm0);
}

unsigned fastEmit_AArch64ISD_MOVIedit_MVT_i32_MVT_v2i64_i_Predicate_imm0_255(uint64_t imm0) {
  if ((Subtarget->hasNEON())) {
    return fastEmitInst_i(AArch64::MOVIv2d_ns, &AArch64::FPR128RegClass, imm0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_MOVIedit_MVT_i32_i_Predicate_imm0_255(MVT RetVT, uint64_t imm0) {
switch (RetVT.SimpleTy) {
  case MVT::f64: return fastEmit_AArch64ISD_MOVIedit_MVT_i32_MVT_f64_i_Predicate_imm0_255(imm0);
  case MVT::v2i64: return fastEmit_AArch64ISD_MOVIedit_MVT_i32_MVT_v2i64_i_Predicate_imm0_255(imm0);
  default: return 0;
}
}

unsigned fastEmit_AArch64ISD_MOVIedit_i_Predicate_imm0_255(MVT VT, MVT RetVT, uint64_t imm0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_AArch64ISD_MOVIedit_MVT_i32_i_Predicate_imm0_255(RetVT, imm0);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_i_Predicate_imm0_255(MVT VT, MVT RetVT, unsigned Opcode, uint64_t imm0) {
  switch (Opcode) {
  case AArch64ISD::FMOV: return fastEmit_AArch64ISD_FMOV_i_Predicate_imm0_255(VT, RetVT, imm0);
  case AArch64ISD::MOVI: return fastEmit_AArch64ISD_MOVI_i_Predicate_imm0_255(VT, RetVT, imm0);
  case AArch64ISD::MOVIedit: return fastEmit_AArch64ISD_MOVIedit_i_Predicate_imm0_255(VT, RetVT, imm0);
  default: return 0;
  }
}

// FastEmit functions for AArch64ISD::RDSVL.

unsigned fastEmit_AArch64ISD_RDSVL_MVT_i32_i_Predicate_simm6_32b(MVT RetVT, uint64_t imm0) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasSME())) {
    return fastEmitInst_i(AArch64::RDSVLI_XI, &AArch64::GPR64RegClass, imm0);
  }
  return 0;
}

unsigned fastEmit_AArch64ISD_RDSVL_i_Predicate_simm6_32b(MVT VT, MVT RetVT, uint64_t imm0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_AArch64ISD_RDSVL_MVT_i32_i_Predicate_simm6_32b(RetVT, imm0);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_i_Predicate_simm6_32b(MVT VT, MVT RetVT, unsigned Opcode, uint64_t imm0) {
  switch (Opcode) {
  case AArch64ISD::RDSVL: return fastEmit_AArch64ISD_RDSVL_i_Predicate_simm6_32b(VT, RetVT, imm0);
  default: return 0;
  }
}

