//===- NaClBitcodeDefs.h ----------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Defines some common types/constants used by bitcode readers and
// writers. It is intended to make clear assumptions made in
// representing bitcode files.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_BITCODE_NACL_NACLBITCODEDEFS_H
#define LLVM_BITCODE_NACL_NACLBITCODEDEFS_H

namespace llvm {

namespace naclbitc {

// Special record codes used to model codes for predefined records.
// They are very large so that they do not conflict with existing
// record codes for user-defined blocks.
enum SpecialBlockCodes {
  BLK_CODE_ENTER = 65535,
  BLK_CODE_EXIT = 65534,
  BLK_CODE_DEFINE_ABBREV = 65533,
  BLK_CODE_HEADER = 65532
};

} // end of namespace naclbitc

/// Defines type for value indicies in bitcode. Like a size_t, but
/// fixed across platforms.
typedef uint32_t NaClBcIndexSize_t;

/// Signed version of NaClBcIndexSize_t. Used to define relative indices.
typedef int32_t NaClRelBcIndexSize_t;

/// Defines maximum allowed bitcode index in bitcode files.
static const size_t NaClBcIndexSize_t_Max =
    std::numeric_limits<NaClBcIndexSize_t>::max();

/// Defines the maximum number of initializers allowed, based on ILP32.
static const size_t MaxNaClGlobalVarInits =
    std::numeric_limits<uint32_t>::max();

} // end of namespace llvm

#endif // LLVM_BITCODE_NACL_NACLBITCODEDEFS_H
