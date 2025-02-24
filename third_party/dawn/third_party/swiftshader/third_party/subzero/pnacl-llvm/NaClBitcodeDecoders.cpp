//===- NaClBitcodeDecoders.cpp --------------------------------------------===//
//     Internal implementation of decoder functions for PNaCl Bitcode files.
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/Bitcode/NaCl/NaClBitcodeDecoders.h"

namespace llvm {
namespace naclbitc {

bool DecodeCastOpcode(uint64_t NaClOpcode, Instruction::CastOps &LLVMOpcode) {
  switch (NaClOpcode) {
  default:
    LLVMOpcode = Instruction::BitCast;
    return false;
  case naclbitc::CAST_TRUNC:
    LLVMOpcode = Instruction::Trunc;
    return true;
  case naclbitc::CAST_ZEXT:
    LLVMOpcode = Instruction::ZExt;
    return true;
  case naclbitc::CAST_SEXT:
    LLVMOpcode = Instruction::SExt;
    return true;
  case naclbitc::CAST_FPTOUI:
    LLVMOpcode = Instruction::FPToUI;
    return true;
  case naclbitc::CAST_FPTOSI:
    LLVMOpcode = Instruction::FPToSI;
    return true;
  case naclbitc::CAST_UITOFP:
    LLVMOpcode = Instruction::UIToFP;
    return true;
  case naclbitc::CAST_SITOFP:
    LLVMOpcode = Instruction::SIToFP;
    return true;
  case naclbitc::CAST_FPTRUNC:
    LLVMOpcode = Instruction::FPTrunc;
    return true;
  case naclbitc::CAST_FPEXT:
    LLVMOpcode = Instruction::FPExt;
    return true;
  case naclbitc::CAST_BITCAST:
    LLVMOpcode = Instruction::BitCast;
    return true;
  }
}

bool DecodeLinkage(uint64_t NaClLinkage,
                   GlobalValue::LinkageTypes &LLVMLinkage) {
  switch (NaClLinkage) {
  default:
    LLVMLinkage = GlobalValue::InternalLinkage;
    return false;
  case naclbitc::LINKAGE_EXTERNAL:
    LLVMLinkage = GlobalValue::ExternalLinkage;
    return true;
  case naclbitc::LINKAGE_INTERNAL:
    LLVMLinkage = GlobalValue::InternalLinkage;
    return true;
  }
}

bool DecodeBinaryOpcode(uint64_t NaClOpcode, Type *Ty,
                        Instruction::BinaryOps &LLVMOpcode) {
  switch (NaClOpcode) {
  default:
    LLVMOpcode = Instruction::Add;
    return false;
  case naclbitc::BINOP_ADD:
    LLVMOpcode = Ty->isFPOrFPVectorTy() ? Instruction::FAdd : Instruction::Add;
    return true;
  case naclbitc::BINOP_SUB:
    LLVMOpcode = Ty->isFPOrFPVectorTy() ? Instruction::FSub : Instruction::Sub;
    return true;
  case naclbitc::BINOP_MUL:
    LLVMOpcode = Ty->isFPOrFPVectorTy() ? Instruction::FMul : Instruction::Mul;
    return true;
  case naclbitc::BINOP_UDIV:
    LLVMOpcode = Instruction::UDiv;
    return true;
  case naclbitc::BINOP_SDIV:
    LLVMOpcode = Ty->isFPOrFPVectorTy() ? Instruction::FDiv : Instruction::SDiv;
    return true;
  case naclbitc::BINOP_UREM:
    LLVMOpcode = Instruction::URem;
    return true;
  case naclbitc::BINOP_SREM:
    LLVMOpcode = Ty->isFPOrFPVectorTy() ? Instruction::FRem : Instruction::SRem;
    return true;
  case naclbitc::BINOP_SHL:
    LLVMOpcode = Instruction::Shl;
    return true;
  case naclbitc::BINOP_LSHR:
    LLVMOpcode = Instruction::LShr;
    return true;
  case naclbitc::BINOP_ASHR:
    LLVMOpcode = Instruction::AShr;
    return true;
  case naclbitc::BINOP_AND:
    LLVMOpcode = Instruction::And;
    return true;
  case naclbitc::BINOP_OR:
    LLVMOpcode = Instruction::Or;
    return true;
  case naclbitc::BINOP_XOR:
    LLVMOpcode = Instruction::Xor;
    return true;
  }
}

bool DecodeCallingConv(uint64_t NaClCallingConv,
                       CallingConv::ID &LLVMCallingConv) {
  switch (NaClCallingConv) {
  default:
    LLVMCallingConv = CallingConv::C;
    return false;
  case naclbitc::C_CallingConv:
    LLVMCallingConv = CallingConv::C;
    return true;
  }
}

bool DecodeFcmpPredicate(uint64_t NaClPredicate,
                         CmpInst::Predicate &LLVMPredicate) {
  switch (NaClPredicate) {
  default:
    LLVMPredicate = CmpInst::FCMP_FALSE;
    return false;
  case naclbitc::FCMP_FALSE:
    LLVMPredicate = CmpInst::FCMP_FALSE;
    return true;
  case naclbitc::FCMP_OEQ:
    LLVMPredicate = CmpInst::FCMP_OEQ;
    return true;
  case naclbitc::FCMP_OGT:
    LLVMPredicate = CmpInst::FCMP_OGT;
    return true;
  case naclbitc::FCMP_OGE:
    LLVMPredicate = CmpInst::FCMP_OGE;
    return true;
  case naclbitc::FCMP_OLT:
    LLVMPredicate = CmpInst::FCMP_OLT;
    return true;
  case naclbitc::FCMP_OLE:
    LLVMPredicate = CmpInst::FCMP_OLE;
    return true;
  case naclbitc::FCMP_ONE:
    LLVMPredicate = CmpInst::FCMP_ONE;
    return true;
  case naclbitc::FCMP_ORD:
    LLVMPredicate = CmpInst::FCMP_ORD;
    return true;
  case naclbitc::FCMP_UNO:
    LLVMPredicate = CmpInst::FCMP_UNO;
    return true;
  case naclbitc::FCMP_UEQ:
    LLVMPredicate = CmpInst::FCMP_UEQ;
    return true;
  case naclbitc::FCMP_UGT:
    LLVMPredicate = CmpInst::FCMP_UGT;
    return true;
  case naclbitc::FCMP_UGE:
    LLVMPredicate = CmpInst::FCMP_UGE;
    return true;
  case naclbitc::FCMP_ULT:
    LLVMPredicate = CmpInst::FCMP_ULT;
    return true;
  case naclbitc::FCMP_ULE:
    LLVMPredicate = CmpInst::FCMP_ULE;
    return true;
  case naclbitc::FCMP_UNE:
    LLVMPredicate = CmpInst::FCMP_UNE;
    return true;
  case naclbitc::FCMP_TRUE:
    LLVMPredicate = CmpInst::FCMP_TRUE;
    return true;
  }
}

bool DecodeIcmpPredicate(uint64_t NaClPredicate,
                         CmpInst::Predicate &LLVMPredicate) {
  switch (NaClPredicate) {
  default:
    LLVMPredicate = CmpInst::ICMP_EQ;
    return false;
  case naclbitc::ICMP_EQ:
    LLVMPredicate = CmpInst::ICMP_EQ;
    return true;
  case naclbitc::ICMP_NE:
    LLVMPredicate = CmpInst::ICMP_NE;
    return true;
  case naclbitc::ICMP_UGT:
    LLVMPredicate = CmpInst::ICMP_UGT;
    return true;
  case naclbitc::ICMP_UGE:
    LLVMPredicate = CmpInst::ICMP_UGE;
    return true;
  case naclbitc::ICMP_ULT:
    LLVMPredicate = CmpInst::ICMP_ULT;
    return true;
  case naclbitc::ICMP_ULE:
    LLVMPredicate = CmpInst::ICMP_ULE;
    return true;
  case naclbitc::ICMP_SGT:
    LLVMPredicate = CmpInst::ICMP_SGT;
    return true;
  case naclbitc::ICMP_SGE:
    LLVMPredicate = CmpInst::ICMP_SGE;
    return true;
  case naclbitc::ICMP_SLT:
    LLVMPredicate = CmpInst::ICMP_SLT;
    return true;
  case naclbitc::ICMP_SLE:
    LLVMPredicate = CmpInst::ICMP_SLE;
    return true;
  }
}

} // namespace naclbitc
} // namespace llvm
