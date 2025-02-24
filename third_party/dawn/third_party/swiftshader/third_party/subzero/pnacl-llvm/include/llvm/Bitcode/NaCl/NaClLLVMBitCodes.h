//===- NaClLLVMBitCodes.h ---------------------------------------*- C++ -*-===//
//     Enum values for the NaCl bitcode wire format
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This header defines Bitcode enum values for NaCl bitcode wire format.
//
// The enum values defined in this file should be considered permanent.  If
// new features are added, they should have values added at the end of the
// respective lists.
//
// Note: PNaCl version 1 is no longer supported, and has been removed from
// comments.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_BITCODE_NACL_NACLLLVMBITCODES_H
#define LLVM_BITCODE_NACL_NACLLLVMBITCODES_H

#include "llvm/Bitcode/NaCl/NaClBitCodes.h"

namespace llvm {
namespace naclbitc {
// The only top-level block type defined is for a module.
enum NaClBlockIDs {
  // Blocks
  MODULE_BLOCK_ID = FIRST_APPLICATION_BLOCKID,

  // Module sub-block id's.
  PARAMATTR_BLOCK_ID,       // Not used in PNaCl.
  PARAMATTR_GROUP_BLOCK_ID, // Not used in PNaCl.

  CONSTANTS_BLOCK_ID,
  FUNCTION_BLOCK_ID,

  UNUSED_ID1,

  VALUE_SYMTAB_BLOCK_ID,
  METADATA_BLOCK_ID,      // Not used in PNaCl.
  METADATA_ATTACHMENT_ID, // Not used in PNaCl.

  TYPE_BLOCK_ID_NEW,

  USELIST_BLOCK_ID, // Not used in PNaCl.
  GLOBALVAR_BLOCK_ID
};

/// MODULE blocks have a number of optional fields and subblocks.
enum NaClModuleCodes {
  MODULE_CODE_VERSION = 1,     // VERSION:     [version#]
  MODULE_CODE_TRIPLE = 2,      // Not used in PNaCl
  MODULE_CODE_DATALAYOUT = 3,  // Not used in PNaCl
  MODULE_CODE_ASM = 4,         // Not used in PNaCl
  MODULE_CODE_SECTIONNAME = 5, // Not used in PNaCl
  MODULE_CODE_DEPLIB = 6,      // Not used in PNaCl
  MODULE_CODE_GLOBALVAR = 7,   // Not used in PNaCl
  // FUNCTION:  [type, callingconv, isproto, linkage]
  MODULE_CODE_FUNCTION = 8,
  MODULE_CODE_ALIAS = 9,      // Not used in PNaCl
  MODULE_CODE_PURGEVALS = 10, // Not used in PNaCl
  MODULE_CODE_GCNAME = 11     // Not used in PNaCl
};

/// PARAMATTR blocks have code for defining a parameter attribute set.
enum NaClAttributeCodes {
  // FIXME: Remove `PARAMATTR_CODE_ENTRY_OLD' in 4.0
  PARAMATTR_CODE_ENTRY_OLD = 1, // ENTRY: [paramidx0, attr0,
                                //         paramidx1, attr1...]
  PARAMATTR_CODE_ENTRY = 2,     // ENTRY: [paramidx0, attrgrp0,
                                //         paramidx1, attrgrp1, ...]
  PARAMATTR_GRP_CODE_ENTRY = 3  // ENTRY: [id, attr0, att1, ...]
};

/// TYPE blocks have codes for each type primitive they use.
enum NaClTypeCodes {
  TYPE_CODE_NUMENTRY = 1, // NUMENTRY: [numentries]

  // Type Codes
  TYPE_CODE_VOID = 2,   // VOID
  TYPE_CODE_FLOAT = 3,  // FLOAT
  TYPE_CODE_DOUBLE = 4, // DOUBLE
  // TODO(mseaborn): Remove LABEL when we drop support for v1 of the
  // PNaCl bitcode format.  The writer no longer generates it.
  TYPE_CODE_LABEL = 5,   // LABEL
  TYPE_CODE_OPAQUE = 6,  // Not used in PNaCl.
  TYPE_CODE_INTEGER = 7, // INTEGER: [width]
  TYPE_CODE_POINTER = 8, // POINTER: [pointee type]

  TYPE_CODE_FUNCTION_OLD = 9, // Not used in PNaCl.

  TYPE_CODE_HALF = 10, // Not used in PNaCl.

  TYPE_CODE_ARRAY = 11,  // Not used in PNaCl.
  TYPE_CODE_VECTOR = 12, // VECTOR: [numelts, eltty]

  // These are not with the other floating point types because they're
  // a late addition, and putting them in the right place breaks
  // binary compatibility.
  TYPE_CODE_X86_FP80 = 13,  // Not used in PNaCl.
  TYPE_CODE_FP128 = 14,     // Not used in PNaCl.
  TYPE_CODE_PPC_FP128 = 15, // Not used in PNaCl.

  TYPE_CODE_METADATA = 16, // Not used in PNaCl.

  TYPE_CODE_X86_MMX = 17, // Not used in PNaCl.

  TYPE_CODE_STRUCT_ANON = 18,  // Not used in PNaCl.
  TYPE_CODE_STRUCT_NAME = 19,  // Not used in PNaCl.
  TYPE_CODE_STRUCT_NAMED = 20, // Not used in PNaCl.

  TYPE_CODE_FUNCTION = 21 // FUNCTION: [vararg, retty, paramty x N]
};

// The type symbol table only has one code (TST_ENTRY_CODE).
enum NaClTypeSymtabCodes {
  TST_CODE_ENTRY = 1 // TST_ENTRY: [typeid, namechar x N]
};

// The value symbol table only has one code (VST_ENTRY_CODE).
enum NaClValueSymtabCodes {
  VST_CODE_ENTRY = 1,  // VST_ENTRY: [valid, namechar x N]
  VST_CODE_BBENTRY = 2 // VST_BBENTRY: [bbid, namechar x N]
};

// Not used in PNaCl.
enum NaClMetadataCodes {
  METADATA_STRING = 1, // MDSTRING:      [values]
  // 2 is unused.
  // 3 is unused.
  METADATA_NAME = 4, // STRING:        [values]
  // 5 is unused.
  METADATA_KIND = 6, // [n x [id, name]]
  // 7 is unused.
  METADATA_NODE = 8,        // NODE:          [n x (type num, value num)]
  METADATA_FN_NODE = 9,     // FN_NODE:       [n x (type num, value num)]
  METADATA_NAMED_NODE = 10, // NAMED_NODE:    [n x mdnodes]
  METADATA_ATTACHMENT = 11  // [m x [value, [n x [id, mdnode]]]
};

// The constants block (CONSTANTS_BLOCK_ID) describes emission for each
// constant and maintains an implicit current type value.
enum NaClConstantsCodes {
  CST_CODE_SETTYPE = 1,          // SETTYPE:       [typeid]
  CST_CODE_NULL = 2,             // Not used in PNaCl.
  CST_CODE_UNDEF = 3,            // UNDEF
  CST_CODE_INTEGER = 4,          // INTEGER:       [intval]
  CST_CODE_WIDE_INTEGER = 5,     // Not used in PNaCl.
  CST_CODE_FLOAT = 6,            // FLOAT:         [fpval]
  CST_CODE_AGGREGATE = 7,        // Not used in PNaCl.
  CST_CODE_STRING = 8,           // Not used in PNaCl.
  CST_CODE_CSTRING = 9,          // Not used in PNaCl.
  CST_CODE_CE_BINOP = 10,        // Not used in PNaCl.
  CST_CODE_CE_CAST = 11,         // Not used in PNaCl.
  CST_CODE_CE_GEP = 12,          // Not used in PNaCl.
  CST_CODE_CE_SELECT = 13,       // Not used in PNaCl.
  CST_CODE_CE_EXTRACTELT = 14,   // Not used in PNaCl.
  CST_CODE_CE_INSERTELT = 15,    // Not used in PNaCl.
  CST_CODE_CE_SHUFFLEVEC = 16,   // Not used in PNaCl.
  CST_CODE_CE_CMP = 17,          // Not used in PNaCl.
  CST_CODE_INLINEASM_OLD = 18,   // No longer used.
  CST_CODE_CE_SHUFVEC_EX = 19,   // Not used in PNaCl.
  CST_CODE_CE_INBOUNDS_GEP = 20, // Not used in PNaCl.
  CST_CODE_BLOCKADDRESS = 21,    // Not used in PNaCl.
  CST_CODE_DATA = 22,            // Not used in PNaCl.
  CST_CODE_INLINEASM = 23        // Not used in PNaCl.
};

/// GlobalVarOpcodes - These are values used in the bitcode files to
/// encode records defining global variables.
///
/// The structure of global variables can be summarized as follows:
///
/// The global variable block begins with a GLOBALVAR_COUNT, defining
/// the number of global variables in the bitcode file. After that,
/// each global variable is defined.
///
/// Global variables are defined by a GLOBALVAR_VAR record, followed
/// by 1 or more records defining its initial value. Simple
/// variables have a single initializer. Structured variables are
/// defined by an initial GLOBALVAR_COMPOUND record defining the
/// number of fields in the structure, followed by an initializer
/// for each of its fields. In this context, a field is either data,
/// or a relocation.  A data field is defined by a
/// GLOBALVAR_ZEROFILL or GLOBALVAR_DATA record.  A relocation field
/// is defined by a GLOBALVAR_RELOC record.
enum NaClGlobalVarOpcodes {
  GLOBALVAR_VAR = 0,      // VAR: [align, isconst]
  GLOBALVAR_COMPOUND = 1, // COMPOUND: [size]
  GLOBALVAR_ZEROFILL = 2, // ZEROFILL: [size]
  GLOBALVAR_DATA = 3,     // DATA: [b0, b1, ...]
  GLOBALVAR_RELOC = 4,    // RELOC: [val, [addend]]
  GLOBALVAR_COUNT = 5     // COUNT: [n]
};

/// CastOpcodes - These are values used in the bitcode files to encode which
/// cast a CST_CODE_CE_CAST or a XXX refers to.  The values of these enums
/// have no fixed relation to the LLVM IR enum values.  Changing these will
/// break compatibility with old files.
enum NaClCastOpcodes {
  CAST_TRUNC = 0,
  CAST_ZEXT = 1,
  CAST_SEXT = 2,
  CAST_FPTOUI = 3,
  CAST_FPTOSI = 4,
  CAST_UITOFP = 5,
  CAST_SITOFP = 6,
  CAST_FPTRUNC = 7,
  CAST_FPEXT = 8,
  // 9 was CAST_PTRTOINT; not used in PNaCl.
  // 10 was CAST_INTTOPTR; not used in PNaCl.
  CAST_BITCAST = 11
};

/// BinaryOpcodes - These are values used in the bitcode files to encode which
/// binop a CST_CODE_CE_BINOP or a XXX refers to.  The values of these enums
/// have no fixed relation to the LLVM IR enum values.  Changing these will
/// break compatibility with old files.
enum NaClBinaryOpcodes {
  BINOP_ADD = 0,
  BINOP_SUB = 1,
  BINOP_MUL = 2,
  BINOP_UDIV = 3,
  BINOP_SDIV = 4, // overloaded for FP
  BINOP_UREM = 5,
  BINOP_SREM = 6, // overloaded for FP
  BINOP_SHL = 7,
  BINOP_LSHR = 8,
  BINOP_ASHR = 9,
  BINOP_AND = 10,
  BINOP_OR = 11,
  BINOP_XOR = 12
};

/// OverflowingBinaryOperatorOptionalFlags - Flags for serializing
/// OverflowingBinaryOperator's SubclassOptionalData contents.
/// Note: This enum is no longer used in PNaCl, because these
/// flags can't exist in files that meet the PNaCl ABI.
enum NaClOverflowingBinaryOperatorOptionalFlags {
  OBO_NO_UNSIGNED_WRAP = 0,
  OBO_NO_SIGNED_WRAP = 1
};

/// PossiblyExactOperatorOptionalFlags - Flags for serializing
/// PossiblyExactOperator's SubclassOptionalData contents.
/// Note: This enum is no longer used in PNaCl, because these
/// flags can't exist in files that meet the PNaCl ABI.
enum NaClPossiblyExactOperatorOptionalFlags { PEO_EXACT = 0 };

/// \brief Flags for serializing floating point binary operators's
/// SubclassOptionalData contents.
/// Note: This enum is no longer used in PNaCl, because these
/// flags shouldn't exist in files that meet the PNaCl ABI, unless
/// they are old. In the latter case, they are ignored by the reader.
enum NaClFloatingPointBinaryOperatorOptionalFlags {
  FPO_UNSAFE_ALGEBRA = 0,
  FPO_NO_NANS = 1,
  FPO_NO_INFS = 2,
  FPO_NO_SIGNED_ZEROS = 3,
  FPO_ALLOW_RECIPROCAL = 4
};

/// Encoded function calling conventions.
enum NaClCallingConventions { C_CallingConv = 0 };

/// Encoded comparison predicates.
enum NaClComparisonPredicates {
  // Opcode              U L G E    Intuitive operation
  FCMP_FALSE = 0, ///< 0 0 0 0    Always false (always folded)
  FCMP_OEQ = 1,   ///< 0 0 0 1    True if ordered and equal
  FCMP_OGT = 2,   ///< 0 0 1 0    True if ordered and greater than
  FCMP_OGE = 3,   ///< 0 0 1 1    True if ordered and greater than or equal
  FCMP_OLT = 4,   ///< 0 1 0 0    True if ordered and less than
  FCMP_OLE = 5,   ///< 0 1 0 1    True if ordered and less than or equal
  FCMP_ONE = 6,   ///< 0 1 1 0    True if ordered and operands are unequal
  FCMP_ORD = 7,   ///< 0 1 1 1    True if ordered (no nans)
  FCMP_UNO = 8,   ///< 1 0 0 0    True if unordered: isnan(X) | isnan(Y)
  FCMP_UEQ = 9,   ///< 1 0 0 1    True if unordered or equal
  FCMP_UGT = 10,  ///< 1 0 1 0    True if unordered or greater than
  FCMP_UGE = 11,  ///< 1 0 1 1    True if unordered, greater than, or equal
  FCMP_ULT = 12,  ///< 1 1 0 0    True if unordered or less than
  FCMP_ULE = 13,  ///< 1 1 0 1    True if unordered, less than, or equal
  FCMP_UNE = 14,  ///< 1 1 1 0    True if unordered or not equal
  FCMP_TRUE = 15, ///< 1 1 1 1    Always true (always folded)
  ICMP_EQ = 32,   ///< equal
  ICMP_NE = 33,   ///< not equal
  ICMP_UGT = 34,  ///< unsigned greater than
  ICMP_UGE = 35,  ///< unsigned greater or equal
  ICMP_ULT = 36,  ///< unsigned less than
  ICMP_ULE = 37,  ///< unsigned less or equal
  ICMP_SGT = 38,  ///< signed greater than
  ICMP_SGE = 39,  ///< signed greater or equal
  ICMP_SLT = 40,  ///< signed less than
  ICMP_SLE = 41   ///< signed less or equal
};

enum NaClLinkageTypes { LINKAGE_EXTERNAL = 0, LINKAGE_INTERNAL = 3 };

// The function body block (FUNCTION_BLOCK_ID) describes function bodies.  It
// can contain a constant block (CONSTANTS_BLOCK_ID).
enum NaClFunctionCodes {
  FUNC_CODE_DECLAREBLOCKS = 1, // DECLAREBLOCKS: [n]

  FUNC_CODE_INST_BINOP = 2,      // BINOP:      [opval, opval, opcode]
                                 // Note: because old PNaCl bitcode files
                                 // may contain flags (which we now ignore),
                                 // the reader must also support:
                                 // BINOP: [opval, opval, opcode, flags]
  FUNC_CODE_INST_CAST = 3,       // CAST:       [opval, destty, castopc]
  FUNC_CODE_INST_GEP = 4,        // Not used in PNaCl.
  FUNC_CODE_INST_SELECT = 5,     // Not used in PNaCl. Replaced by VSELECT.
  FUNC_CODE_INST_EXTRACTELT = 6, // EXTRACTELT: [opval, opval]
  FUNC_CODE_INST_INSERTELT = 7,  // INSERTELT:  [opval, opval, opval]
  FUNC_CODE_INST_SHUFFLEVEC = 8, // Not used in PNaCl.
  FUNC_CODE_INST_CMP = 9,        // Not used in PNaCl. Replaced by CMP2.
  FUNC_CODE_INST_RET = 10,       // RET:        [opval<optional>]
  FUNC_CODE_INST_BR = 11,        // BR:         [bb#, bb#, cond] or [bb#]
  FUNC_CODE_INST_SWITCH = 12,    // SWITCH:     [opty, op0, op1, ...]
  FUNC_CODE_INST_INVOKE = 13,    // Not used in PNaCl.
  // 14 is unused.
  FUNC_CODE_INST_UNREACHABLE = 15, // UNREACHABLE

  FUNC_CODE_INST_PHI = 16, // PHI:        [ty, val0,bb0, ...]
  // 17 is unused.
  // 18 is unused.
  FUNC_CODE_INST_ALLOCA = 19, // ALLOCA:     [op, align]
  FUNC_CODE_INST_LOAD = 20,   // LOAD: [op, align, ty]
  // 21 is unused.
  // 22 is unused.
  FUNC_CODE_INST_VAARG = 23, // Not used in PNaCl.
  FUNC_CODE_INST_STORE = 24, // STORE: [ptr, val, align]
  // 25 is unused.
  FUNC_CODE_INST_EXTRACTVAL = 26, // Not used in PNaCl.
  FUNC_CODE_INST_INSERTVAL = 27,  // Not used in PNaCl.
  // fcmp/icmp returning Int1TY or vector of Int1Ty. Same as CMP, exists to
  // support legacy vicmp/vfcmp instructions.
  FUNC_CODE_INST_CMP2 = 28, // CMP2:       [opval, opval, pred]
  // new select on i1 or [N x i1]
  FUNC_CODE_INST_VSELECT = 29,      // VSELECT:    [opval, opval, pred]
  FUNC_CODE_INST_INBOUNDS_GEP = 30, // Not used in PNaCl.
  FUNC_CODE_INST_INDIRECTBR = 31,   // Not used in PNaCl.
  // 32 is unused.
  FUNC_CODE_DEBUG_LOC_AGAIN = 33, // Not used in PNaCl.

  FUNC_CODE_INST_CALL = 34,           // CALL: [cc, fnid, args...]
                                      // See FUNC_CODE_INST_CALL_INDIRECT below.
  FUNC_CODE_DEBUG_LOC = 35,           // Not used in PNaCl.
  FUNC_CODE_INST_FENCE = 36,          // Not used in PNaCl.
  FUNC_CODE_INST_CMPXCHG = 37,        // Not used in PNaCl.
  FUNC_CODE_INST_ATOMICRMW = 38,      // Not used in PNaCl.
  FUNC_CODE_INST_RESUME = 39,         // Not used in PNaCl.
  FUNC_CODE_INST_LANDINGPAD = 40,     // Not used in PNaCl.
  FUNC_CODE_INST_LOADATOMIC = 41,     // Not used in PNaCl.
  FUNC_CODE_INST_STOREATOMIC = 42,    // Not used in PNaCl.
  FUNC_CODE_INST_FORWARDTYPEREF = 43, // TYPE: [opval, ty]
  // CALL_INDIRECT: [cc, fnid, returnty, args...]
  FUNC_CODE_INST_CALL_INDIRECT = 44
};
} // namespace naclbitc
} // namespace llvm

#endif
