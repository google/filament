//===- NaClBitcodeDecoders.h -------------------------------------*- C++
//-*-===//
//     Functions used to decode values in PNaCl bitcode files.
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This header file provides a public API for value decoders defined in
// the PNaCl bitcode reader.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_BITCODE_NACL_NACLBITCODEDECODERS_H
#define LLVM_BITCODE_NACL_NACLBITCODEDECODERS_H

#include "llvm/Bitcode/NaCl/NaClLLVMBitCodes.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"

namespace llvm {
namespace naclbitc {

/// Converts the NaCl (bitcode file) cast opcode to the corresponding
/// LLVM cast opcode. Returns true if the conversion
/// succeeds. Otherwise sets LLVMOpcode to Instruction::BitCast and
/// returns false.
bool DecodeCastOpcode(uint64_t NaClOpcode, Instruction::CastOps &LLVMOpcode);

/// Converts the NaCl (bitcode file) linkage type to the corresponding
/// LLVM linkage type. Returns true if the conversion
/// succeeds. Otherwise sets LLVMLinkage to
/// GlobalValue::InternalLinkage and returns false.
bool DecodeLinkage(uint64_t NaClLinkage,
                   GlobalValue::LinkageTypes &LLVMLinkage);

/// Converts the NaCl (bitcode file) binary opcode to the
/// corresponding LLVM binary opcode, assuming that the operator
/// operates on OpType.  Returns true if the conversion
/// succeeds. Otherwise sets LLVMOpcode to Instruction::Add and
/// returns false.
bool DecodeBinaryOpcode(uint64_t NaClOpcode, Type *OpType,
                        Instruction::BinaryOps &LLVMOpcode);

/// Converts the NaCl (bitcode file) calling convention value to the
/// corresponding LLVM calling conventions. Returns true if the
/// conversion succeeds. Otherwise sets LLVMCallingConv to
/// CallingConv::C and returns false.
bool DecodeCallingConv(uint64_t NaClCallingConv,
                       CallingConv::ID &LLVMCallingConv);

/// Converts the NaCl (bitcode file) float comparison predicate to the
/// corresponding LLVM float comparison predicate. Returns true if the
/// conversion succeeds. Otherwise sets LLVMPredicate to
/// CmpInst::FCMP_FALSE and returns false.
bool DecodeFcmpPredicate(uint64_t NaClPredicate,
                         CmpInst::Predicate &LLVMPredicate);

/// Converts the NaCl (bitcode file) integer comparison predicate to
/// the corresponding LLVM integer comparison predicate. Returns true
/// if the conversion succeeds. Otherwise sets LLVMPredicate to
/// CmpInst::ICMP_EQ and returns false.
bool DecodeIcmpPredicate(uint64_t NaClPredicate,
                         CmpInst::Predicate &LLVMPredicate);

} // namespace naclbitc
} // namespace llvm

#endif
