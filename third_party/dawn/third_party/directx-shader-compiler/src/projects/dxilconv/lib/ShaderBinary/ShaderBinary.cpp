///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// ShaderBinary.cpp                                                          //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Vertex shader binary format parsing and encoding.                         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// HLSL change start
#include "ShaderBinaryIncludes.h"
#include "llvm/Support/Compiler.h" // for LLVM_FALLTHROUGH
// HLSL change end

/*==========================================================================;
 *
 *  D3D10ShaderBinary namespace
 *
 ***************************************************************************/

namespace D3D10ShaderBinary {

BOOL IsOpCodeValid(D3D10_SB_OPCODE_TYPE OpCode) {
  return OpCode < D3D10_SB_NUM_OPCODES;
}

UINT GetNumInstructionOperands(D3D10_SB_OPCODE_TYPE OpCode) {
  if (IsOpCodeValid(OpCode))
    return g_InstructionInfo[OpCode].m_NumOperands;
  else
    throw E_FAIL;
}

CInstructionInfo g_InstructionInfo[D3D10_SB_NUM_OPCODES];

void InitInstructionInfo() {
#define SET(OpCode, Name, NumOperands, PrecMask, OpClass)                      \
  (g_InstructionInfo[OpCode].Set(NumOperands, Name, OpClass, PrecMask))

  SET(D3D10_SB_OPCODE_ADD, "add", 3, 0x06, D3D10_SB_FLOAT_OP);
  SET(D3D10_SB_OPCODE_AND, "and", 3, 0x06, D3D10_SB_BIT_OP);
  SET(D3D10_SB_OPCODE_BREAK, "break", 0, 0x00, D3D10_SB_FLOW_OP);
  SET(D3D10_SB_OPCODE_BREAKC, "breakc", 1, 0x00, D3D10_SB_FLOW_OP);
  SET(D3D10_SB_OPCODE_CALL, "call", 1, 0x00, D3D10_SB_FLOW_OP);
  SET(D3D10_SB_OPCODE_CALLC, "callc", 2, 0x00, D3D10_SB_FLOW_OP);
  SET(D3D10_SB_OPCODE_CONTINUE, "continue", 0, 0x00, D3D10_SB_FLOW_OP);
  SET(D3D10_SB_OPCODE_CONTINUEC, "continuec", 1, 0x00, D3D10_SB_FLOW_OP);
  SET(D3D10_SB_OPCODE_CASE, "case", 1, 0x00, D3D10_SB_FLOW_OP);
  SET(D3D10_SB_OPCODE_CUT, "cut", 0, 0x00, D3D10_SB_FLOW_OP);
  SET(D3D10_SB_OPCODE_DEFAULT, "default", 0, 0x00, D3D10_SB_FLOW_OP);
  SET(D3D10_SB_OPCODE_DISCARD, "discard", 1, 0x00, D3D10_SB_FLOW_OP);
  SET(D3D10_SB_OPCODE_DIV, "div", 3, 0x06, D3D10_SB_FLOAT_OP);
  SET(D3D10_SB_OPCODE_DP2, "dp2", 3, 0x06, D3D10_SB_FLOAT_OP);
  SET(D3D10_SB_OPCODE_DP3, "dp3", 3, 0x06, D3D10_SB_FLOAT_OP);
  SET(D3D10_SB_OPCODE_DP4, "dp4", 3, 0x06, D3D10_SB_FLOAT_OP);
  SET(D3D10_SB_OPCODE_ELSE, "else", 0, 0x00, D3D10_SB_FLOW_OP);
  SET(D3D10_SB_OPCODE_EMIT, "emit", 0, 0x00, D3D10_SB_FLOW_OP);
  SET(D3D10_SB_OPCODE_EMITTHENCUT, "emit_then_cut", 0, 0x00, D3D10_SB_FLOW_OP);
  SET(D3D10_SB_OPCODE_ENDIF, "endif", 0, 0x00, D3D10_SB_FLOW_OP);
  SET(D3D10_SB_OPCODE_ENDLOOP, "endloop", 0, 0x00, D3D10_SB_FLOW_OP);
  SET(D3D10_SB_OPCODE_ENDSWITCH, "endswitch", 0, 0x00, D3D10_SB_FLOW_OP);
  SET(D3D10_SB_OPCODE_EQ, "eq", 3, 0x00, D3D10_SB_FLOAT_OP);
  SET(D3D10_SB_OPCODE_EXP, "exp", 2, 0x02, D3D10_SB_FLOAT_OP);
  SET(D3D10_SB_OPCODE_FRC, "frc", 2, 0x02, D3D10_SB_FLOAT_OP);
  SET(D3D10_SB_OPCODE_FTOI, "ftoi", 2, 0x00, D3D10_SB_FLOAT_OP);
  SET(D3D10_SB_OPCODE_FTOU, "ftou", 2, 0x00, D3D10_SB_FLOAT_OP);
  SET(D3D10_SB_OPCODE_GE, "ge", 3, 0x00, D3D10_SB_FLOAT_OP);
  SET(D3D10_SB_OPCODE_DERIV_RTX, "deriv_rtx", 2, 0x02, D3D10_SB_FLOAT_OP);
  SET(D3D10_SB_OPCODE_DERIV_RTY, "deriv_rty", 2, 0x02, D3D10_SB_FLOAT_OP);
  SET(D3D10_SB_OPCODE_IADD, "iadd", 3, 0x06, D3D10_SB_INT_OP);
  SET(D3D10_SB_OPCODE_IF, "if", 1, 0x00, D3D10_SB_FLOW_OP);
  SET(D3D10_SB_OPCODE_IEQ, "ieq", 3, 0x00, D3D10_SB_INT_OP);
  SET(D3D10_SB_OPCODE_IGE, "ige", 3, 0x00, D3D10_SB_INT_OP);
  SET(D3D10_SB_OPCODE_ILT, "ilt", 3, 0x00, D3D10_SB_INT_OP);
  SET(D3D10_SB_OPCODE_IMAD, "imad", 4, 0x0e, D3D10_SB_INT_OP);
  SET(D3D10_SB_OPCODE_IMAX, "imax", 3, 0x06, D3D10_SB_INT_OP);
  SET(D3D10_SB_OPCODE_IMIN, "imin", 3, 0x06, D3D10_SB_INT_OP);
  SET(D3D10_SB_OPCODE_IMUL, "imul", 4, 0x0c, D3D10_SB_INT_OP);
  SET(D3D10_SB_OPCODE_INE, "ine", 3, 0x00, D3D10_SB_INT_OP);
  SET(D3D10_SB_OPCODE_INEG, "ineg", 2, 0x02, D3D10_SB_INT_OP);
  SET(D3D10_SB_OPCODE_ISHL, "ishl", 3, 0x02, D3D10_SB_INT_OP);
  SET(D3D10_SB_OPCODE_ISHR, "ishr", 3, 0x02, D3D10_SB_INT_OP);
  SET(D3D10_SB_OPCODE_ITOF, "itof", 2, 0x00, D3D10_SB_INT_OP);
  SET(D3D10_SB_OPCODE_LABEL, "label", 1, 0x00, D3D10_SB_FLOW_OP);
  SET(D3D10_SB_OPCODE_LD, "ld", 3, 0x00, D3D10_SB_TEX_OP);
  SET(D3D10_SB_OPCODE_LD_MS, "ldms", 4, 0x00, D3D10_SB_TEX_OP);
  SET(D3D10_SB_OPCODE_LOG, "log", 2, 0x02, D3D10_SB_FLOAT_OP);
  SET(D3D10_SB_OPCODE_LOOP, "loop", 0, 0x00, D3D10_SB_FLOW_OP);
  SET(D3D10_SB_OPCODE_LT, "lt", 3, 0x00, D3D10_SB_FLOAT_OP);
  SET(D3D10_SB_OPCODE_MAD, "mad", 4, 0x0e, D3D10_SB_FLOAT_OP);
  SET(D3D10_SB_OPCODE_MAX, "max", 3, 0x06, D3D10_SB_FLOAT_OP);
  SET(D3D10_SB_OPCODE_MIN, "min", 3, 0x06, D3D10_SB_FLOAT_OP);
  SET(D3D10_SB_OPCODE_MOV, "mov", 2, 0x02, D3D10_SB_FLOAT_OP);
  SET(D3D10_SB_OPCODE_MOVC, "movc", 4, 0x0c, D3D10_SB_FLOAT_OP);
  SET(D3D10_SB_OPCODE_MUL, "mul", 3, 0x06, D3D10_SB_FLOAT_OP);
  SET(D3D10_SB_OPCODE_NE, "ne", 3, 0x00, D3D10_SB_FLOAT_OP);
  SET(D3D10_SB_OPCODE_NOP, "nop", 0, 0x00, D3D10_SB_FLOW_OP);
  SET(D3D10_SB_OPCODE_NOT, "not", 2, 0x02, D3D10_SB_BIT_OP);
  SET(D3D10_SB_OPCODE_OR, "or", 3, 0x06, D3D10_SB_BIT_OP);
  SET(D3D10_SB_OPCODE_RESINFO, "resinfo", 3, 0x00, D3D10_SB_TEX_OP);
  SET(D3D10_SB_OPCODE_RET, "ret", 0, 0x00, D3D10_SB_FLOW_OP);
  SET(D3D10_SB_OPCODE_RETC, "retc", 1, 0x00, D3D10_SB_FLOW_OP);
  SET(D3D10_SB_OPCODE_ROUND_NE, "round_ne", 2, 0x02, D3D10_SB_FLOAT_OP);
  SET(D3D10_SB_OPCODE_ROUND_NI, "round_ni", 2, 0x02, D3D10_SB_FLOAT_OP);
  SET(D3D10_SB_OPCODE_ROUND_PI, "round_pi", 2, 0x02, D3D10_SB_FLOAT_OP);
  SET(D3D10_SB_OPCODE_ROUND_Z, "round_z", 2, 0x02, D3D10_SB_FLOAT_OP);
  SET(D3D10_SB_OPCODE_RSQ, "rsq", 2, 0x02, D3D10_SB_FLOAT_OP);
  SET(D3D10_SB_OPCODE_SAMPLE, "sample", 4, 0x00, D3D10_SB_TEX_OP);
  SET(D3D10_SB_OPCODE_SAMPLE_B, "sample_b", 5, 0x00, D3D10_SB_TEX_OP);
  SET(D3D10_SB_OPCODE_SAMPLE_L, "sample_l", 5, 0x00, D3D10_SB_TEX_OP);
  SET(D3D10_SB_OPCODE_SAMPLE_D, "sample_d", 6, 0x00, D3D10_SB_TEX_OP);
  SET(D3D10_SB_OPCODE_SAMPLE_C, "sample_c", 5, 0x00, D3D10_SB_TEX_OP);
  SET(D3D10_SB_OPCODE_SAMPLE_C_LZ, "sample_c_lz", 5, 0x00, D3D10_SB_TEX_OP);
  SET(D3D10_SB_OPCODE_SQRT, "sqrt", 2, 0x02, D3D10_SB_FLOAT_OP);
  SET(D3D10_SB_OPCODE_SWITCH, "switch", 1, 0x00, D3D10_SB_FLOW_OP);
  SET(D3D10_SB_OPCODE_SINCOS, "sincos", 3, 0x04, D3D10_SB_FLOAT_OP);
  SET(D3D10_SB_OPCODE_UDIV, "udiv", 4, 0x0c, D3D10_SB_UINT_OP);
  SET(D3D10_SB_OPCODE_ULT, "ult", 3, 0x00, D3D10_SB_UINT_OP);
  SET(D3D10_SB_OPCODE_UGE, "uge", 3, 0x00, D3D10_SB_UINT_OP);
  SET(D3D10_SB_OPCODE_UMAX, "umax", 3, 0x06, D3D10_SB_UINT_OP);
  SET(D3D10_SB_OPCODE_UMIN, "umin", 3, 0x06, D3D10_SB_UINT_OP);
  SET(D3D10_SB_OPCODE_UMUL, "umul", 4, 0x0c, D3D10_SB_UINT_OP);
  SET(D3D10_SB_OPCODE_UMAD, "umad", 4, 0x0e, D3D10_SB_UINT_OP);
  SET(D3D10_SB_OPCODE_USHR, "ushr", 3, 0x02, D3D10_SB_UINT_OP);
  SET(D3D10_SB_OPCODE_UTOF, "utof", 2, 0x00, D3D10_SB_UINT_OP);
  SET(D3D10_SB_OPCODE_XOR, "xor", 3, 0x06, D3D10_SB_BIT_OP);
  SET(D3D10_SB_OPCODE_RESERVED0, "jmp", 0, 0x00, D3D10_SB_FLOW_OP);
  SET(D3D10_SB_OPCODE_DCL_INPUT, "dcl_input", 1, 0x00, D3D10_SB_DCL_OP);
  SET(D3D10_SB_OPCODE_DCL_OUTPUT, "dcl_output", 1, 0x00, D3D10_SB_DCL_OP);
  SET(D3D10_SB_OPCODE_DCL_INPUT_SGV, "dcl_input_sgv", 1, 0x00, D3D10_SB_DCL_OP);
  SET(D3D10_SB_OPCODE_DCL_INPUT_PS_SGV, "dcl_input_ps_sgv", 1, 0x00,
      D3D10_SB_DCL_OP);
  SET(D3D10_SB_OPCODE_DCL_GS_INPUT_PRIMITIVE, "dcl_inputprimitive", 0, 0x00,
      D3D10_SB_DCL_OP);
  SET(D3D10_SB_OPCODE_DCL_GS_OUTPUT_PRIMITIVE_TOPOLOGY, "dcl_outputtopology", 0,
      0x00, D3D10_SB_DCL_OP);
  SET(D3D10_SB_OPCODE_DCL_MAX_OUTPUT_VERTEX_COUNT, "dcl_maxout", 0, 0x00,
      D3D10_SB_DCL_OP);
  SET(D3D10_SB_OPCODE_DCL_INPUT_PS, "dcl_input_ps", 1, 0x00, D3D10_SB_DCL_OP);
  SET(D3D10_SB_OPCODE_DCL_CONSTANT_BUFFER, "dcl_constantbuffer", 1, 0x00,
      D3D10_SB_DCL_OP);
  SET(D3D10_SB_OPCODE_DCL_SAMPLER, "dcl_sampler", 1, 0x00, D3D10_SB_DCL_OP);
  SET(D3D10_SB_OPCODE_DCL_RESOURCE, "dcl_resource", 1, 0x00, D3D10_SB_DCL_OP);
  SET(D3D10_SB_OPCODE_DCL_INPUT_SIV, "dcl_input_siv", 1, 0x00, D3D10_SB_DCL_OP);
  SET(D3D10_SB_OPCODE_DCL_INPUT_PS_SIV, "dcl_input_ps_siv", 1, 0x00,
      D3D10_SB_DCL_OP);
  SET(D3D10_SB_OPCODE_DCL_OUTPUT_SIV, "dcl_output_siv", 1, 0x00,
      D3D10_SB_DCL_OP);
  SET(D3D10_SB_OPCODE_DCL_OUTPUT_SGV, "dcl_output_sgv", 1, 0x00,
      D3D10_SB_DCL_OP);
  SET(D3D10_SB_OPCODE_DCL_TEMPS, "dcl_temps", 0, 0x00, D3D10_SB_DCL_OP);
  SET(D3D10_SB_OPCODE_DCL_INDEXABLE_TEMP, "dcl_indexableTemp", 0, 0x00,
      D3D10_SB_DCL_OP);
  SET(D3D10_SB_OPCODE_DCL_INDEX_RANGE, "dcl_indexrange", 1, 0x00,
      D3D10_SB_DCL_OP);
  SET(D3D10_SB_OPCODE_DCL_GLOBAL_FLAGS, "dcl_globalFlags", 0, 0x00,
      D3D10_SB_DCL_OP);

  SET(D3D10_1_SB_OPCODE_SAMPLE_INFO, "sampleinfo", 2, 0x00, D3D10_SB_TEX_OP);
  SET(D3D10_1_SB_OPCODE_SAMPLE_POS, "samplepos", 3, 0x00, D3D10_SB_TEX_OP);
  SET(D3D10_1_SB_OPCODE_GATHER4, "gather4", 4, 0x00, D3D10_SB_TEX_OP);
  SET(D3D10_1_SB_OPCODE_LOD, "lod", 4, 0x00, D3D10_SB_TEX_OP);

  SET(D3D11_SB_OPCODE_EMIT_STREAM, "emit_stream", 1, 0x00, D3D10_SB_FLOW_OP);
  SET(D3D11_SB_OPCODE_CUT_STREAM, "cut_stream", 1, 0x00, D3D10_SB_FLOW_OP);
  SET(D3D11_SB_OPCODE_EMITTHENCUT_STREAM, "emit_then_cut_stream", 1, 0x00,
      D3D10_SB_FLOW_OP);
  SET(D3D11_SB_OPCODE_INTERFACE_CALL, "fcall", 1, 0x00, D3D10_SB_FLOW_OP);

  SET(D3D11_SB_OPCODE_DCL_STREAM, "dcl_stream", 1, 0x00, D3D10_SB_DCL_OP);
  SET(D3D11_SB_OPCODE_DCL_FUNCTION_BODY, "dcl_function_body", 0, 0x00,
      D3D10_SB_DCL_OP);
  SET(D3D11_SB_OPCODE_DCL_FUNCTION_TABLE, "dcl_function_table", 0, 0x00,
      D3D10_SB_DCL_OP);
  SET(D3D11_SB_OPCODE_DCL_INTERFACE, "dcl_interface", 0, 0x00, D3D10_SB_DCL_OP);

  SET(D3D11_SB_OPCODE_BUFINFO, "bufinfo", 2, 0x00, D3D10_SB_TEX_OP);
  SET(D3D11_SB_OPCODE_DERIV_RTX_COARSE, "deriv_rtx_coarse", 2, 0x02,
      D3D10_SB_FLOAT_OP);
  SET(D3D11_SB_OPCODE_DERIV_RTX_FINE, "deriv_rtx_fine", 2, 0x02,
      D3D10_SB_FLOAT_OP);
  SET(D3D11_SB_OPCODE_DERIV_RTY_COARSE, "deriv_rty_coarse", 2, 0x02,
      D3D10_SB_FLOAT_OP);
  SET(D3D11_SB_OPCODE_DERIV_RTY_FINE, "deriv_rty_fine", 2, 0x02,
      D3D10_SB_FLOAT_OP);
  SET(D3D11_SB_OPCODE_GATHER4_C, "gather4_c", 5, 0x00, D3D10_SB_TEX_OP);
  SET(D3D11_SB_OPCODE_GATHER4_PO, "gather4_po", 5, 0x00, D3D10_SB_TEX_OP);
  SET(D3D11_SB_OPCODE_GATHER4_PO_C, "gather4_po_c", 6, 0x00, D3D10_SB_TEX_OP);
  SET(D3D11_SB_OPCODE_RCP, "rcp", 2, 0x02, D3D10_SB_FLOAT_OP);
  SET(D3D11_SB_OPCODE_F32TOF16, "f32tof16", 2, 0x00, D3D10_SB_FLOAT_OP);
  SET(D3D11_SB_OPCODE_F16TOF32, "f16tof32", 2, 0x00, D3D10_SB_FLOAT_OP);
  SET(D3D11_SB_OPCODE_UADDC, "uaddc", 4, 0x0c, D3D10_SB_UINT_OP);
  SET(D3D11_SB_OPCODE_USUBB, "usubb", 4, 0x0c, D3D10_SB_UINT_OP);
  SET(D3D11_SB_OPCODE_COUNTBITS, "countbits", 2, 0x02, D3D10_SB_BIT_OP);
  SET(D3D11_SB_OPCODE_FIRSTBIT_HI, "firstbit_hi", 2, 0x02, D3D10_SB_BIT_OP);
  SET(D3D11_SB_OPCODE_FIRSTBIT_LO, "firstbit_lo", 2, 0x02, D3D10_SB_BIT_OP);
  SET(D3D11_SB_OPCODE_FIRSTBIT_SHI, "firstbit_shi", 2, 0x02, D3D10_SB_BIT_OP);
  SET(D3D11_SB_OPCODE_UBFE, "ubfe", 4, 0x02, D3D10_SB_BIT_OP);
  SET(D3D11_SB_OPCODE_IBFE, "ibfe", 4, 0x02, D3D10_SB_BIT_OP);
  SET(D3D11_SB_OPCODE_BFI, "bfi", 5, 0x02, D3D10_SB_BIT_OP);
  SET(D3D11_SB_OPCODE_BFREV, "bfrev", 2, 0x02, D3D10_SB_BIT_OP);
  SET(D3D11_SB_OPCODE_SWAPC, "swapc", 5, 0x02, D3D10_SB_FLOAT_OP);

  SET(D3D11_SB_OPCODE_HS_DECLS, "hs_decls", 0, 0x00, D3D10_SB_DCL_OP);
  SET(D3D11_SB_OPCODE_HS_CONTROL_POINT_PHASE, "hs_control_point_phase", 0, 0x00,
      D3D10_SB_DCL_OP);
  SET(D3D11_SB_OPCODE_HS_FORK_PHASE, "hs_fork_phase", 0, 0x00, D3D10_SB_DCL_OP);
  SET(D3D11_SB_OPCODE_HS_JOIN_PHASE, "hs_join_phase", 0, 0x00, D3D10_SB_DCL_OP);

  SET(D3D11_SB_OPCODE_DCL_INPUT_CONTROL_POINT_COUNT,
      "dcl_input_control_point_count", 0, 0x00, D3D10_SB_DCL_OP);
  SET(D3D11_SB_OPCODE_DCL_OUTPUT_CONTROL_POINT_COUNT,
      "dcl_output_control_point_count", 0, 0x00, D3D10_SB_DCL_OP);
  SET(D3D11_SB_OPCODE_DCL_TESS_DOMAIN, "dcl_tessellator_domain", 0, 0x00,
      D3D10_SB_DCL_OP);
  SET(D3D11_SB_OPCODE_DCL_TESS_PARTITIONING, "dcl_tessellator_partitioning", 0,
      0x00, D3D10_SB_DCL_OP);
  SET(D3D11_SB_OPCODE_DCL_TESS_OUTPUT_PRIMITIVE,
      "dcl_tessellator_output_primitive", 0, 0x00, D3D10_SB_DCL_OP);
  SET(D3D11_SB_OPCODE_DCL_HS_MAX_TESSFACTOR, "dcl_hs_max_tessfactor", 0, 0x00,
      D3D10_SB_DCL_OP);
  SET(D3D11_SB_OPCODE_DCL_HS_FORK_PHASE_INSTANCE_COUNT,
      "dcl_hs_fork_phase_instance_count", 0, 0x00, D3D10_SB_DCL_OP);
  SET(D3D11_SB_OPCODE_DCL_HS_JOIN_PHASE_INSTANCE_COUNT,
      "dcl_hs_join_phase_instance_count", 0, 0x00, D3D10_SB_DCL_OP);

  SET(D3D11_SB_OPCODE_DCL_THREAD_GROUP, "dcl_thread_group", 0, 0x00,
      D3D10_SB_DCL_OP);
  SET(D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_TYPED, "dcl_uav_typed", 1, 0x00,
      D3D10_SB_DCL_OP);
  SET(D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_RAW, "dcl_uav_raw", 1, 0x00,
      D3D10_SB_DCL_OP);
  SET(D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_STRUCTURED,
      "dcl_uav_structured", 1, 0x00, D3D10_SB_DCL_OP);
  SET(D3D11_SB_OPCODE_DCL_THREAD_GROUP_SHARED_MEMORY_RAW, "dcl_tgsm_raw", 1,
      0x00, D3D10_SB_DCL_OP);
  SET(D3D11_SB_OPCODE_DCL_THREAD_GROUP_SHARED_MEMORY_STRUCTURED,
      "dcl_tgsm_structured", 1, 0x00, D3D10_SB_DCL_OP);
  SET(D3D11_SB_OPCODE_DCL_RESOURCE_RAW, "dcl_resource_raw", 1, 0x00,
      D3D10_SB_DCL_OP);
  SET(D3D11_SB_OPCODE_DCL_RESOURCE_STRUCTURED, "dcl_resource_structured", 1,
      0x00, D3D10_SB_DCL_OP);
  SET(D3D11_SB_OPCODE_LD_UAV_TYPED, "ld_uav_typed", 3, 0x00, D3D11_SB_MEM_OP);
  SET(D3D11_SB_OPCODE_STORE_UAV_TYPED, "store_uav_typed", 3, 0x00,
      D3D11_SB_MEM_OP);
  SET(D3D11_SB_OPCODE_LD_RAW, "ld_raw", 3, 0x00, D3D11_SB_MEM_OP);
  SET(D3D11_SB_OPCODE_STORE_RAW, "store_raw", 3, 0x00, D3D11_SB_MEM_OP);
  SET(D3D11_SB_OPCODE_LD_STRUCTURED, "ld_structured", 4, 0x00, D3D11_SB_MEM_OP);
  SET(D3D11_SB_OPCODE_STORE_STRUCTURED, "store_structured", 4, 0x00,
      D3D11_SB_MEM_OP);
  SET(D3D11_SB_OPCODE_ATOMIC_AND, "atomic_and", 3, 0x00, D3D11_SB_ATOMIC_OP);
  SET(D3D11_SB_OPCODE_ATOMIC_OR, "atomic_or", 3, 0x00, D3D11_SB_ATOMIC_OP);
  SET(D3D11_SB_OPCODE_ATOMIC_XOR, "atomic_xor", 3, 0x00, D3D11_SB_ATOMIC_OP);
  SET(D3D11_SB_OPCODE_ATOMIC_CMP_STORE, "atomic_cmp_store", 4, 0x00,
      D3D11_SB_ATOMIC_OP);
  SET(D3D11_SB_OPCODE_ATOMIC_IADD, "atomic_iadd", 3, 0x00, D3D11_SB_ATOMIC_OP);
  SET(D3D11_SB_OPCODE_ATOMIC_IMAX, "atomic_imax", 3, 0x00, D3D11_SB_ATOMIC_OP);
  SET(D3D11_SB_OPCODE_ATOMIC_IMIN, "atomic_imin", 3, 0x00, D3D11_SB_ATOMIC_OP);
  SET(D3D11_SB_OPCODE_ATOMIC_UMAX, "atomic_umax", 3, 0x00, D3D11_SB_ATOMIC_OP);
  SET(D3D11_SB_OPCODE_ATOMIC_UMIN, "atomic_umin", 3, 0x00, D3D11_SB_ATOMIC_OP);
  SET(D3D11_SB_OPCODE_IMM_ATOMIC_ALLOC, "imm_atomic_alloc", 2, 0x00,
      D3D11_SB_ATOMIC_OP);
  SET(D3D11_SB_OPCODE_IMM_ATOMIC_CONSUME, "imm_atomic_consume", 2, 0x00,
      D3D11_SB_ATOMIC_OP);
  SET(D3D11_SB_OPCODE_IMM_ATOMIC_IADD, "imm_atomic_iadd", 4, 0x00,
      D3D11_SB_ATOMIC_OP);
  SET(D3D11_SB_OPCODE_IMM_ATOMIC_AND, "imm_atomic_and", 4, 0x00,
      D3D11_SB_ATOMIC_OP);
  SET(D3D11_SB_OPCODE_IMM_ATOMIC_OR, "imm_atomic_or", 4, 0x00,
      D3D11_SB_ATOMIC_OP);
  SET(D3D11_SB_OPCODE_IMM_ATOMIC_XOR, "imm_atomic_xor", 4, 0x00,
      D3D11_SB_ATOMIC_OP);
  SET(D3D11_SB_OPCODE_IMM_ATOMIC_EXCH, "imm_atomic_exch", 4, 0x00,
      D3D11_SB_ATOMIC_OP);
  SET(D3D11_SB_OPCODE_IMM_ATOMIC_CMP_EXCH, "imm_atomic_cmp_exch", 5, 0x00,
      D3D11_SB_ATOMIC_OP);
  SET(D3D11_SB_OPCODE_IMM_ATOMIC_IMAX, "imm_atomic_imax", 4, 0x00,
      D3D11_SB_ATOMIC_OP);
  SET(D3D11_SB_OPCODE_IMM_ATOMIC_IMIN, "imm_atomic_imin", 4, 0x00,
      D3D11_SB_ATOMIC_OP);
  SET(D3D11_SB_OPCODE_IMM_ATOMIC_UMAX, "imm_atomic_umax", 4, 0x00,
      D3D11_SB_ATOMIC_OP);
  SET(D3D11_SB_OPCODE_IMM_ATOMIC_UMIN, "imm_atomic_umin", 4, 0x00,
      D3D11_SB_ATOMIC_OP);
  SET(D3D11_SB_OPCODE_SYNC, "sync", 0, 0x00, D3D10_SB_FLOW_OP);
  SET(D3D11_SB_OPCODE_EVAL_SNAPPED, "eval_snapped", 3, 0x02, D3D10_SB_FLOAT_OP);
  SET(D3D11_SB_OPCODE_EVAL_SAMPLE_INDEX, "eval_sample_index", 3, 0x02,
      D3D10_SB_FLOAT_OP);
  SET(D3D11_SB_OPCODE_EVAL_CENTROID, "eval_centroid", 2, 0x02,
      D3D10_SB_FLOAT_OP);

  SET(D3D11_SB_OPCODE_DCL_GS_INSTANCE_COUNT, "dcl_gsinstances", 0, 0x00,
      D3D10_SB_DCL_OP);

  SET(D3D11_SB_OPCODE_DADD, "dadd", 3, 0x06, D3D11_SB_DOUBLE_OP);
  SET(D3D11_SB_OPCODE_DMAX, "dmax", 3, 0x06, D3D11_SB_DOUBLE_OP);
  SET(D3D11_SB_OPCODE_DMIN, "dmin", 3, 0x06, D3D11_SB_DOUBLE_OP);
  SET(D3D11_SB_OPCODE_DMUL, "dmul", 3, 0x06, D3D11_SB_DOUBLE_OP);
  SET(D3D11_SB_OPCODE_DEQ, "deq", 3, 0x00, D3D11_SB_DOUBLE_OP);
  SET(D3D11_SB_OPCODE_DGE, "dge", 3, 0x00, D3D11_SB_DOUBLE_OP);
  SET(D3D11_SB_OPCODE_DLT, "dlt", 3, 0x00, D3D11_SB_DOUBLE_OP);
  SET(D3D11_SB_OPCODE_DNE, "dne", 3, 0x00, D3D11_SB_DOUBLE_OP);
  SET(D3D11_SB_OPCODE_DMOV, "dmov", 2, 0x02, D3D11_SB_DOUBLE_OP);
  SET(D3D11_SB_OPCODE_DMOVC, "dmovc", 4, 0x0c, D3D11_SB_DOUBLE_OP);
  SET(D3D11_SB_OPCODE_DTOF, "dtof", 2, 0x02, D3D11_SB_DOUBLE_TO_FLOAT_OP);
  SET(D3D11_SB_OPCODE_FTOD, "ftod", 2, 0x00, D3D11_SB_FLOAT_TO_DOUBLE_OP);

  SET(D3D11_SB_OPCODE_ABORT, "abort", 0, 0x00, D3D11_SB_DEBUG_OP);
  SET(D3D11_SB_OPCODE_DEBUG_BREAK, "debug_break", 0, 0x00, D3D11_SB_DEBUG_OP);

  SET(D3D11_1_SB_OPCODE_DDIV, "ddiv", 3, 0x06, D3D11_SB_DOUBLE_OP);
  SET(D3D11_1_SB_OPCODE_DFMA, "dfma", 4, 0x0e, D3D11_SB_DOUBLE_OP);
  SET(D3D11_1_SB_OPCODE_DRCP, "drcp", 2, 0x02, D3D11_SB_DOUBLE_OP);

  SET(D3D11_1_SB_OPCODE_MSAD, "msad", 4, 0x0e, D3D10_SB_UINT_OP);

  SET(D3D11_1_SB_OPCODE_DTOI, "dtoi", 2, 0x00, D3D11_SB_DOUBLE_OP);
  SET(D3D11_1_SB_OPCODE_DTOU, "dtou", 2, 0x00, D3D11_SB_DOUBLE_OP);
  SET(D3D11_1_SB_OPCODE_ITOD, "itod", 2, 0x00, D3D10_SB_INT_OP);
  SET(D3D11_1_SB_OPCODE_UTOD, "utod", 2, 0x00, D3D10_SB_UINT_OP);

  SET(D3DWDDM1_3_SB_OPCODE_GATHER4_FEEDBACK, "gather4_s", 5, 0x00,
      D3D10_SB_TEX_OP);
  SET(D3DWDDM1_3_SB_OPCODE_GATHER4_C_FEEDBACK, "gather4_c_s", 6, 0x00,
      D3D10_SB_TEX_OP);
  SET(D3DWDDM1_3_SB_OPCODE_GATHER4_PO_FEEDBACK, "gather4_po_s", 6, 0x00,
      D3D10_SB_TEX_OP);
  SET(D3DWDDM1_3_SB_OPCODE_GATHER4_PO_C_FEEDBACK, "gather4_po_c_s", 7, 0x00,
      D3D10_SB_TEX_OP);
  SET(D3DWDDM1_3_SB_OPCODE_LD_FEEDBACK, "ld_s", 4, 0x00, D3D10_SB_TEX_OP);
  SET(D3DWDDM1_3_SB_OPCODE_LD_MS_FEEDBACK, "ldms_s", 5, 0x00, D3D10_SB_TEX_OP);
  SET(D3DWDDM1_3_SB_OPCODE_LD_UAV_TYPED_FEEDBACK, "ld_uav_typed_s", 4, 0x00,
      D3D11_SB_MEM_OP);
  SET(D3DWDDM1_3_SB_OPCODE_LD_RAW_FEEDBACK, "ld_raw_s", 4, 0x00,
      D3D11_SB_MEM_OP);
  SET(D3DWDDM1_3_SB_OPCODE_LD_STRUCTURED_FEEDBACK, "ld_structured_s", 5, 0x00,
      D3D11_SB_MEM_OP);
  SET(D3DWDDM1_3_SB_OPCODE_SAMPLE_L_FEEDBACK, "sample_l_s", 6, 0x00,
      D3D10_SB_TEX_OP);
  SET(D3DWDDM1_3_SB_OPCODE_SAMPLE_C_LZ_FEEDBACK, "sample_c_lz_s", 6, 0x00,
      D3D10_SB_TEX_OP);
  SET(D3DWDDM1_3_SB_OPCODE_SAMPLE_CLAMP_FEEDBACK, "sample_cl_s", 6, 0x00,
      D3D10_SB_TEX_OP);
  SET(D3DWDDM1_3_SB_OPCODE_SAMPLE_B_CLAMP_FEEDBACK, "sample_b_cl_s", 7, 0x00,
      D3D10_SB_TEX_OP);
  SET(D3DWDDM1_3_SB_OPCODE_SAMPLE_D_CLAMP_FEEDBACK, "sample_d_cl_s", 8, 0x00,
      D3D10_SB_TEX_OP);
  SET(D3DWDDM1_3_SB_OPCODE_SAMPLE_C_CLAMP_FEEDBACK, "sample_c_cl_s", 7, 0x00,
      D3D10_SB_TEX_OP);
  SET(D3DWDDM1_3_SB_OPCODE_CHECK_ACCESS_FULLY_MAPPED,
      "check_access_fully_mapped", 2, 0x00, D3D10_SB_TEX_OP);
}

//*****************************************************************************
//
//  CShaderCodeParser
//
//*****************************************************************************

void CShaderCodeParser::SetShader(CONST CShaderToken *pBuffer) {
  m_pShaderCode = pBuffer;
  m_pShaderEndToken = pBuffer + pBuffer[1];
  // First OpCode token
  m_pCurrentToken = &pBuffer[2];
}

D3D10_SB_TOKENIZED_PROGRAM_TYPE CShaderCodeParser::ShaderType() {
  return (D3D10_SB_TOKENIZED_PROGRAM_TYPE)
      DECODE_D3D10_SB_TOKENIZED_PROGRAM_TYPE(*m_pShaderCode);
}

UINT CShaderCodeParser::CurrentTokenOffset() {
  return (UINT)(m_pCurrentToken - m_pShaderCode);
}

void CShaderCodeParser::SetCurrentTokenOffset(UINT Offset) {
  m_pCurrentToken = m_pShaderCode + Offset;
}

UINT CShaderCodeParser::ShaderLengthInTokens() { return m_pShaderCode[1]; }

UINT CShaderCodeParser::ShaderMinorVersion() {
  return DECODE_D3D10_SB_TOKENIZED_PROGRAM_MINOR_VERSION(m_pShaderCode[0]);
}

UINT CShaderCodeParser::ShaderMajorVersion() {
  return DECODE_D3D10_SB_TOKENIZED_PROGRAM_MAJOR_VERSION(m_pShaderCode[0]);
}

void CShaderCodeParser::ParseIndex(
    COperandIndex *pOperandIndex,
    D3D10_SB_OPERAND_INDEX_REPRESENTATION IndexType) {
  switch (IndexType) {
  case D3D10_SB_OPERAND_INDEX_IMMEDIATE32:
    pOperandIndex->m_RegIndex = *m_pCurrentToken++;
    pOperandIndex->m_ComponentName = D3D10_SB_4_COMPONENT_X;
    pOperandIndex->m_RelRegType = D3D10_SB_OPERAND_TYPE_IMMEDIATE32;
    break;
  case D3D10_SB_OPERAND_INDEX_IMMEDIATE64:
    pOperandIndex->m_RegIndexA[0] = *m_pCurrentToken++;
    pOperandIndex->m_RegIndexA[1] = *m_pCurrentToken++;
    pOperandIndex->m_ComponentName = D3D10_SB_4_COMPONENT_X;
    pOperandIndex->m_RelRegType = D3D10_SB_OPERAND_TYPE_IMMEDIATE64;
    break;
  case D3D10_SB_OPERAND_INDEX_RELATIVE: {
    COperand operand;
    ParseOperand(&operand);
    pOperandIndex->m_RelIndex = operand.m_Index[0].m_RegIndex;
    pOperandIndex->m_RelIndex1 = operand.m_Index[1].m_RegIndex;
    pOperandIndex->m_RelRegType = operand.m_Type;
    pOperandIndex->m_IndexDimension = operand.m_IndexDimension;
    pOperandIndex->m_ComponentName = operand.m_ComponentName;
    pOperandIndex->m_MinPrecision = operand.m_MinPrecision;
    break;
  }
  case D3D10_SB_OPERAND_INDEX_IMMEDIATE32_PLUS_RELATIVE: {
    pOperandIndex->m_RegIndex = *m_pCurrentToken++;
    COperand operand;
    ParseOperand(&operand);
    pOperandIndex->m_RelIndex = operand.m_Index[0].m_RegIndex;
    pOperandIndex->m_RelIndex1 = operand.m_Index[1].m_RegIndex;
    pOperandIndex->m_RelRegType = operand.m_Type;
    pOperandIndex->m_IndexDimension = operand.m_IndexDimension;
    pOperandIndex->m_ComponentName = operand.m_ComponentName;
    pOperandIndex->m_MinPrecision = operand.m_MinPrecision;
  } break;
  default:
    throw E_FAIL;
  }
}

void CShaderCodeParser::ParseOperand(COperandBase *pOperand) {
  CShaderToken Token = *m_pCurrentToken++;

  pOperand->m_Type = DECODE_D3D10_SB_OPERAND_TYPE(Token);
  pOperand->m_NumComponents = DECODE_D3D10_SB_OPERAND_NUM_COMPONENTS(Token);
  pOperand->m_bExtendedOperand = DECODE_IS_D3D10_SB_OPERAND_EXTENDED(Token);

  UINT NumComponents = 0;
  switch (pOperand->m_NumComponents) {
  case D3D10_SB_OPERAND_1_COMPONENT:
    NumComponents = 1;
    break;
  case D3D10_SB_OPERAND_4_COMPONENT:
    NumComponents = 4;
    break;
  }

  switch (pOperand->m_Type) {
  case D3D10_SB_OPERAND_TYPE_IMMEDIATE32:
  case D3D10_SB_OPERAND_TYPE_IMMEDIATE64:
    break;
  default: {
    if (pOperand->m_NumComponents == D3D10_SB_OPERAND_4_COMPONENT) {
      // Component selection mode
      pOperand->m_ComponentSelection =
          DECODE_D3D10_SB_OPERAND_4_COMPONENT_SELECTION_MODE(Token);
      switch (pOperand->m_ComponentSelection) {
      case D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE:
        pOperand->m_WriteMask = DECODE_D3D10_SB_OPERAND_4_COMPONENT_MASK(Token);
        break;
      case D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE:
        pOperand->m_Swizzle[0] =
            DECODE_D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_SOURCE(Token, 0);
        pOperand->m_Swizzle[1] =
            DECODE_D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_SOURCE(Token, 1);
        pOperand->m_Swizzle[2] =
            DECODE_D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_SOURCE(Token, 2);
        pOperand->m_Swizzle[3] =
            DECODE_D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_SOURCE(Token, 3);
        break;
      case D3D10_SB_OPERAND_4_COMPONENT_SELECT_1_MODE: {
        D3D10_SB_4_COMPONENT_NAME Component =
            DECODE_D3D10_SB_OPERAND_4_COMPONENT_SELECT_1(Token);
        pOperand->m_Swizzle[0] = static_cast<BYTE>(Component);
        pOperand->m_Swizzle[1] = static_cast<BYTE>(Component);
        pOperand->m_Swizzle[2] = static_cast<BYTE>(Component);
        pOperand->m_Swizzle[3] = static_cast<BYTE>(Component);
        pOperand->m_ComponentName = Component;
        break;
      }
      default:
        throw E_FAIL;
      }
    }
    pOperand->m_IndexDimension = DECODE_D3D10_SB_OPERAND_INDEX_DIMENSION(Token);
    if (pOperand->m_IndexDimension != D3D10_SB_OPERAND_INDEX_0D) {
      UINT NumDimensions = pOperand->m_IndexDimension;
      // Index representation
      for (UINT i = 0; i < NumDimensions; i++) {
        pOperand->m_IndexType[i] =
            DECODE_D3D10_SB_OPERAND_INDEX_REPRESENTATION(i, Token);
      }
    }
    break;
  }
  }

  // Extended operand
  if (pOperand->m_bExtendedOperand) {
    Token = *m_pCurrentToken++;
    pOperand->m_ExtendedOperandType =
        DECODE_D3D10_SB_EXTENDED_OPERAND_TYPE(Token);
    if (pOperand->m_ExtendedOperandType == D3D10_SB_EXTENDED_OPERAND_MODIFIER) {
      pOperand->m_Modifier = DECODE_D3D10_SB_OPERAND_MODIFIER(Token);
      pOperand->m_MinPrecision = DECODE_D3D11_SB_OPERAND_MIN_PRECISION(Token);
      pOperand->m_Nonuniform = DECODE_D3D12_SB_OPERAND_NON_UNIFORM(Token);
    }
  }

  switch (pOperand->m_Type) {
  case D3D10_SB_OPERAND_TYPE_IMMEDIATE32:
  case D3D10_SB_OPERAND_TYPE_IMMEDIATE64:
    for (UINT i = 0; i < NumComponents; i++) {
      pOperand->m_Value[i] = *m_pCurrentToken++;
    }
    break;
  }

  // Operand indices
  if (pOperand->m_IndexDimension != D3D10_SB_OPERAND_INDEX_0D) {
    const UINT NumDimensions = pOperand->m_IndexDimension;
    // Index representation
    for (UINT i = 0; i < NumDimensions; i++) {
      ParseIndex(&pOperand->m_Index[i], pOperand->m_IndexType[i]);
    }
  }
}

void CShaderCodeParser::ParseInstruction(CInstruction *pInstruction) {
  pInstruction->Clear(true);
  const CShaderToken *pStart = m_pCurrentToken;
  const CShaderToken Token = *m_pCurrentToken++;
  pInstruction->m_OpCode = DECODE_D3D10_SB_OPCODE_TYPE(Token);
  pInstruction->m_PreciseMask =
      DECODE_D3D11_SB_INSTRUCTION_PRECISE_VALUES(Token);
  pInstruction->m_bSaturate =
      DECODE_IS_D3D10_SB_INSTRUCTION_SATURATE_ENABLED(Token);
  UINT InstructionLength = DECODE_D3D10_SB_TOKENIZED_INSTRUCTION_LENGTH(Token);
  pInstruction->m_NumOperands =
      GetNumInstructionOperands(pInstruction->m_OpCode);
  BOOL b51PlusShader =
      (ShaderMajorVersion() > 5 ||
       (ShaderMajorVersion() == 5 && ShaderMinorVersion() > 0));
  BOOL bExtended = DECODE_IS_D3D10_SB_OPCODE_EXTENDED(Token);
  if (bExtended &&
      ((pInstruction->m_OpCode == D3D11_SB_OPCODE_DCL_INTERFACE) ||
       (pInstruction->m_OpCode == D3D11_SB_OPCODE_DCL_FUNCTION_TABLE))) {
    pInstruction->m_ExtendedOpCodeCount = 1;
#pragma prefast(suppress                                                       \
                : __WARNING_LOCALDECLHIDESLOCAL,                               \
                  "This uses the same variable name for continuity.")
    CShaderToken Token = *m_pCurrentToken++;
    // these instructions may be longer than can fit in the normal
    // instructionlength field
    InstructionLength = (UINT)(Token);
  } else {
    pInstruction->m_ExtendedOpCodeCount = 0;
    for (int i = 0;
         i < (bExtended ? D3D11_SB_MAX_SIMULTANEOUS_EXTENDED_OPCODES : 0);
         i++) {
      pInstruction->m_ExtendedOpCodeCount++;
#pragma prefast(suppress                                                       \
                : __WARNING_LOCALDECLHIDESLOCAL,                               \
                  "This uses the same variable name for continuity.")
      CShaderToken Token = *m_pCurrentToken++;
      bExtended = DECODE_IS_D3D10_SB_OPCODE_EXTENDED(Token);
      pInstruction->m_OpCodeEx[i] = DECODE_D3D10_SB_EXTENDED_OPCODE_TYPE(Token);
      switch (pInstruction->m_OpCodeEx[i]) {
      case D3D10_SB_EXTENDED_OPCODE_SAMPLE_CONTROLS: {
        pInstruction->m_TexelOffset[0] =
            (INT8)DECODE_IMMEDIATE_D3D10_SB_ADDRESS_OFFSET(
                D3D10_SB_IMMEDIATE_ADDRESS_OFFSET_U, Token);
        pInstruction->m_TexelOffset[1] =
            (INT8)DECODE_IMMEDIATE_D3D10_SB_ADDRESS_OFFSET(
                D3D10_SB_IMMEDIATE_ADDRESS_OFFSET_V, Token);
        pInstruction->m_TexelOffset[2] =
            (INT8)DECODE_IMMEDIATE_D3D10_SB_ADDRESS_OFFSET(
                D3D10_SB_IMMEDIATE_ADDRESS_OFFSET_W, Token);
        for (UINT i = 0; i < 3; i++) {
          if (pInstruction->m_TexelOffset[i] & 0x8)
            pInstruction->m_TexelOffset[i] |= 0xfffffff0;
        }
        break;
      } break;
      case D3D11_SB_EXTENDED_OPCODE_RESOURCE_DIM: {
        pInstruction->m_ResourceDimEx =
            DECODE_D3D11_SB_EXTENDED_RESOURCE_DIMENSION(Token);
        pInstruction->m_ResourceDimStructureStrideEx =
            DECODE_D3D11_SB_EXTENDED_RESOURCE_DIMENSION_STRUCTURE_STRIDE(Token);
      } break;
      case D3D11_SB_EXTENDED_OPCODE_RESOURCE_RETURN_TYPE: {
        for (UINT j = 0; j < 4; j++) {
          pInstruction->m_ResourceReturnTypeEx[j] =
              DECODE_D3D11_SB_EXTENDED_RESOURCE_RETURN_TYPE(Token, j);
        }
      } break;
      }
      if (!bExtended) {
        break;
      }
    }
  }
  switch (pInstruction->m_OpCode) {
  case D3D10_SB_OPCODE_CUSTOMDATA:
    pInstruction->m_PreciseMask = 0;
    pInstruction->m_bSaturate = false;
    pInstruction->m_NumOperands = 0;

    // not bothering to keep custom-data for now. TODO: store
    pInstruction->m_CustomData.Type = DECODE_D3D10_SB_CUSTOMDATA_CLASS(Token);
    InstructionLength = *m_pCurrentToken;
    if (*m_pCurrentToken < 2) {
      InstructionLength = 2;
      pInstruction->m_CustomData.pData = 0;
      pInstruction->m_CustomData.DataSizeInBytes = 0;
    } else {
      pInstruction->m_CustomData.DataSizeInBytes = (*m_pCurrentToken - 2) * 4;
      pInstruction->m_CustomData.pData =
          malloc((*m_pCurrentToken - 2) * sizeof(UINT));
      if (NULL == pInstruction->m_CustomData.pData) {
        throw E_OUTOFMEMORY;
      }
      memcpy(pInstruction->m_CustomData.pData, m_pCurrentToken + 1,
             (*m_pCurrentToken - 2) * 4);

      switch (pInstruction->m_CustomData.Type) {
      case D3D11_SB_CUSTOMDATA_SHADER_MESSAGE: {
        CShaderMessage *pMessage = &pInstruction->m_CustomData.ShaderMessage;
        UINT Length = pInstruction->m_CustomData.DataSizeInBytes / 4;
        UINT *pData = (UINT *)pInstruction->m_CustomData.pData;

        ZeroMemory(pMessage, sizeof(*pMessage));

        if (Length < 6) {
          break;
        }

        UINT StrChars = pData[2];
        // Add one for the terminator and then round up.
        UINT StrWords = (StrChars + sizeof(DWORD)) / sizeof(DWORD);
        UINT NumOperands = pData[3];
        UINT OpLength = pData[4];

        // Enforce some basic sanity size limits.
        if (OpLength >= 0x10000 || NumOperands >= 0x1000 ||
            StrWords >= 0x10000 || Length < 5 + OpLength + StrWords) {
          break;
        }

        UINT *pOpEnd = &pData[5 + OpLength];

        pMessage->pOperands =
            (COperand *)malloc(NumOperands * sizeof(COperand));
        if (!pMessage->pOperands) {
          throw E_OUTOFMEMORY;
        }

        CONST CShaderToken *pOperands = (CShaderToken *)&pData[5];
        for (UINT i = 0; i < NumOperands; i++) {
          if (pOperands >= pOpEnd) {
            break;
          }

          pMessage->pOperands[i].Clear();
          pOperands =
              ParseOperandAt(&pMessage->pOperands[i], pOperands, pOpEnd);
        }
        if (pOperands != pOpEnd) {
          free(pMessage->pOperands);
          pMessage->pOperands = NULL;
          break;
        }

        // Now that we're sure everything is valid we can
        // fill in the message info.
        pMessage->MessageID = (D3D11_SB_SHADER_MESSAGE_ID)pData[0];
        pMessage->FormatStyle = (D3D11_SB_SHADER_MESSAGE_FORMAT)pData[1];
        pMessage->pFormatString = (PCSTR)pOpEnd;
        pMessage->NumOperands = NumOperands;
        break;
      }
      case D3D10_SB_CUSTOMDATA_COMMENT: {
        // Guarantee that the C string comment is Null-terminated
        *((LPSTR)pInstruction->m_CustomData.pData +
          pInstruction->m_CustomData.DataSizeInBytes - 1) = '\0';
        break;
      }
      }
    }
    break;
  case D3D11_SB_OPCODE_DCL_FUNCTION_BODY:
    pInstruction->m_FunctionBodyDecl.FunctionBodyNumber =
        (UINT)(*m_pCurrentToken);
    m_pCurrentToken++;
    break;
  case D3D11_SB_OPCODE_DCL_FUNCTION_TABLE:
    pInstruction->m_FunctionTableDecl.FunctionTableNumber =
        (UINT)(*m_pCurrentToken);
    m_pCurrentToken++;
    pInstruction->m_FunctionTableDecl.TableLength = (UINT)(*m_pCurrentToken);

    // opcode
    // instruction length if extended instruction
    // table ID
    // table length
    // data
    assert(InstructionLength ==
           (3 + (bExtended ? 1 : 0) +
            pInstruction->m_FunctionTableDecl.TableLength));

    pInstruction->m_FunctionTableDecl.pFunctionIdentifiers = (UINT *)malloc(
        pInstruction->m_FunctionTableDecl.TableLength * sizeof(UINT));

    if (NULL == pInstruction->m_FunctionTableDecl.pFunctionIdentifiers) {
      throw E_OUTOFMEMORY;
    }

    m_pCurrentToken++;

    memcpy(pInstruction->m_FunctionTableDecl.pFunctionIdentifiers,
           m_pCurrentToken,
           pInstruction->m_FunctionTableDecl.TableLength * sizeof(UINT));
    break;
  case D3D11_SB_OPCODE_DCL_INTERFACE:
    pInstruction->m_InterfaceDecl.bDynamicallyIndexed =
        DECODE_D3D11_SB_INTERFACE_INDEXED_BIT(Token);
    pInstruction->m_InterfaceDecl.InterfaceNumber = (WORD)(*m_pCurrentToken);
    m_pCurrentToken++;
    pInstruction->m_InterfaceDecl.ExpectedTableSize = (UINT)(*m_pCurrentToken);
    m_pCurrentToken++;
    // there's a limit of 64k types, so that gives a max length on this table.
    pInstruction->m_InterfaceDecl.TableLength =
        DECODE_D3D11_SB_INTERFACE_TABLE_LENGTH(*m_pCurrentToken);
    // this puts a limit on the size of interface arrays at 64k
    pInstruction->m_InterfaceDecl.ArrayLength =
        DECODE_D3D11_SB_INTERFACE_ARRAY_LENGTH(*m_pCurrentToken);

    // opcode
    // instruction length if extended instruction
    // interface ID
    // table size
    // num types/array length
    // data
    assert(InstructionLength == (4 + (bExtended ? 1 : 0) +
                                 pInstruction->m_InterfaceDecl.TableLength));

    pInstruction->m_InterfaceDecl.pFunctionTableIdentifiers = (UINT *)malloc(
        pInstruction->m_InterfaceDecl.TableLength * sizeof(UINT));

    if (NULL == pInstruction->m_InterfaceDecl.pFunctionTableIdentifiers) {
      throw E_OUTOFMEMORY;
    }

    m_pCurrentToken++;

    memcpy(pInstruction->m_InterfaceDecl.pFunctionTableIdentifiers,
           m_pCurrentToken,
           pInstruction->m_InterfaceDecl.TableLength * sizeof(UINT));
    break;
  case D3D11_SB_OPCODE_INTERFACE_CALL:
    pInstruction->m_InterfaceCall.FunctionIndex = *m_pCurrentToken++;
    pInstruction->m_InterfaceCall.pInterfaceOperand = pInstruction->m_Operands;
    ParseOperand(pInstruction->m_InterfaceCall.pInterfaceOperand);
    break;
  case D3D10_SB_OPCODE_DCL_RESOURCE:
    pInstruction->m_ResourceDecl.Dimension =
        DECODE_D3D10_SB_RESOURCE_DIMENSION(Token);
    ParseOperand(&pInstruction->m_Operands[0]);
    pInstruction->m_ResourceDecl.ReturnType[0] =
        DECODE_D3D10_SB_RESOURCE_RETURN_TYPE(*m_pCurrentToken, 0);
    pInstruction->m_ResourceDecl.ReturnType[1] =
        DECODE_D3D10_SB_RESOURCE_RETURN_TYPE(*m_pCurrentToken, 1);
    pInstruction->m_ResourceDecl.ReturnType[2] =
        DECODE_D3D10_SB_RESOURCE_RETURN_TYPE(*m_pCurrentToken, 2);
    pInstruction->m_ResourceDecl.ReturnType[3] =
        DECODE_D3D10_SB_RESOURCE_RETURN_TYPE(*m_pCurrentToken, 3);
    pInstruction->m_ResourceDecl.SampleCount =
        DECODE_D3D10_SB_RESOURCE_SAMPLE_COUNT(Token);
    m_pCurrentToken++;
    pInstruction->m_ResourceDecl.Space = 0;
    if (b51PlusShader) {
      pInstruction->m_ResourceDecl.Space = (UINT)(*m_pCurrentToken++);
    }
    break;

  case D3D10_SB_OPCODE_DCL_SAMPLER:
    pInstruction->m_SamplerDecl.SamplerMode =
        DECODE_D3D10_SB_SAMPLER_MODE(Token);
    ParseOperand(&pInstruction->m_Operands[0]);
    pInstruction->m_SamplerDecl.Space = 0;
    if (b51PlusShader) {
      pInstruction->m_SamplerDecl.Space = (UINT)(*m_pCurrentToken++);
    }
    break;

  case D3D11_SB_OPCODE_DCL_STREAM:
    ParseOperand(&pInstruction->m_Operands[0]);
    break;

  case D3D10_SB_OPCODE_DCL_TEMPS:
    pInstruction->m_TempsDecl.NumTemps = (UINT)(*m_pCurrentToken);
    m_pCurrentToken++;
    break;

  case D3D10_SB_OPCODE_DCL_INDEXABLE_TEMP:
    pInstruction->m_IndexableTempDecl.IndexableTempNumber =
        (UINT)(*m_pCurrentToken);
    m_pCurrentToken++;
    pInstruction->m_IndexableTempDecl.NumRegisters = (UINT)(*m_pCurrentToken);
    m_pCurrentToken++;
    switch (min(4u, max(1u, (UINT)(*m_pCurrentToken)))) {
    case 1:
      pInstruction->m_IndexableTempDecl.Mask =
          D3D10_SB_OPERAND_4_COMPONENT_MASK_X;
      break;
    case 2:
      pInstruction->m_IndexableTempDecl.Mask =
          D3D10_SB_OPERAND_4_COMPONENT_MASK_X |
          D3D10_SB_OPERAND_4_COMPONENT_MASK_Y;
      break;
    case 3:
      pInstruction->m_IndexableTempDecl.Mask =
          D3D10_SB_OPERAND_4_COMPONENT_MASK_X |
          D3D10_SB_OPERAND_4_COMPONENT_MASK_Y |
          D3D10_SB_OPERAND_4_COMPONENT_MASK_Z;
      break;
    case 4:
      pInstruction->m_IndexableTempDecl.Mask =
          D3D10_SB_OPERAND_4_COMPONENT_MASK_ALL;
      break;
    }
    m_pCurrentToken++;
    break;

  case D3D10_SB_OPCODE_DCL_INPUT:
  case D3D10_SB_OPCODE_DCL_OUTPUT:
    ParseOperand(&pInstruction->m_Operands[0]);
    break;

  case D3D10_SB_OPCODE_DCL_INPUT_SIV:
    ParseOperand(&pInstruction->m_Operands[0]);
    pInstruction->m_InputDeclSIV.Name = DECODE_D3D10_SB_NAME(*m_pCurrentToken);
    m_pCurrentToken++;
    break;

  case D3D10_SB_OPCODE_DCL_INPUT_SGV:
    ParseOperand(&pInstruction->m_Operands[0]);
    pInstruction->m_InputDeclSIV.Name = DECODE_D3D10_SB_NAME(*m_pCurrentToken);
    m_pCurrentToken++;
    break;

  case D3D10_SB_OPCODE_DCL_INPUT_PS:
    pInstruction->m_InputPSDecl.InterpolationMode =
        DECODE_D3D10_SB_INPUT_INTERPOLATION_MODE(Token);
    ParseOperand(&pInstruction->m_Operands[0]);
    break;

  case D3D10_SB_OPCODE_DCL_INPUT_PS_SIV:
    pInstruction->m_InputPSDeclSIV.InterpolationMode =
        DECODE_D3D10_SB_INPUT_INTERPOLATION_MODE(Token);
    ParseOperand(&pInstruction->m_Operands[0]);
    pInstruction->m_InputPSDeclSIV.Name =
        DECODE_D3D10_SB_NAME(*m_pCurrentToken);
    m_pCurrentToken++;
    break;

  case D3D10_SB_OPCODE_DCL_INPUT_PS_SGV:
    pInstruction->m_InputPSDeclSGV.InterpolationMode =
        DECODE_D3D10_SB_INPUT_INTERPOLATION_MODE(Token);
    ParseOperand(&pInstruction->m_Operands[0]);
    pInstruction->m_InputPSDeclSGV.Name =
        DECODE_D3D10_SB_NAME(*m_pCurrentToken);
    m_pCurrentToken++;
    break;

  case D3D10_SB_OPCODE_DCL_OUTPUT_SIV:
    ParseOperand(&pInstruction->m_Operands[0]);
    pInstruction->m_OutputDeclSIV.Name = DECODE_D3D10_SB_NAME(*m_pCurrentToken);
    m_pCurrentToken++;
    break;

  case D3D10_SB_OPCODE_DCL_OUTPUT_SGV:
    ParseOperand(&pInstruction->m_Operands[0]);
    pInstruction->m_OutputDeclSGV.Name = DECODE_D3D10_SB_NAME(*m_pCurrentToken);
    m_pCurrentToken++;
    break;

  case D3D10_SB_OPCODE_DCL_INDEX_RANGE:
    ParseOperand(&pInstruction->m_Operands[0]);
    pInstruction->m_IndexRangeDecl.RegCount = (UINT)(*m_pCurrentToken);
    m_pCurrentToken++;
    break;

  case D3D10_SB_OPCODE_DCL_CONSTANT_BUFFER:
    pInstruction->m_ConstantBufferDecl.AccessPattern =
        DECODE_D3D10_SB_CONSTANT_BUFFER_ACCESS_PATTERN(Token);
    ParseOperand(&pInstruction->m_Operands[0]);
    pInstruction->m_ConstantBufferDecl.Space = 0;
    if (b51PlusShader) {
      pInstruction->m_ConstantBufferDecl.Size = (UINT)(*m_pCurrentToken++);
      pInstruction->m_ConstantBufferDecl.Space = (UINT)(*m_pCurrentToken++);
    } else {
      pInstruction->m_ConstantBufferDecl.Size =
          pInstruction->m_Operands[0].m_Index[1].m_RegIndex;
    }
    break;

  case D3D10_SB_OPCODE_DCL_GS_OUTPUT_PRIMITIVE_TOPOLOGY:
    pInstruction->m_OutputTopologyDecl.Topology =
        DECODE_D3D10_SB_GS_OUTPUT_PRIMITIVE_TOPOLOGY(Token);
    break;

  case D3D10_SB_OPCODE_DCL_GS_INPUT_PRIMITIVE:
    pInstruction->m_InputPrimitiveDecl.Primitive =
        DECODE_D3D10_SB_GS_INPUT_PRIMITIVE(Token);
    break;

  case D3D10_SB_OPCODE_DCL_MAX_OUTPUT_VERTEX_COUNT:
    pInstruction->m_GSMaxOutputVertexCountDecl.MaxOutputVertexCount =
        (UINT)(*m_pCurrentToken);
    m_pCurrentToken++;
    break;

  case D3D11_SB_OPCODE_DCL_GS_INSTANCE_COUNT:
    pInstruction->m_GSInstanceCountDecl.InstanceCount =
        (UINT)(*m_pCurrentToken);
    m_pCurrentToken++;
    break;

  case D3D10_SB_OPCODE_DCL_GLOBAL_FLAGS:
    pInstruction->m_GlobalFlagsDecl.Flags = DECODE_D3D10_SB_GLOBAL_FLAGS(Token);
    break;

  case D3D11_SB_OPCODE_DCL_INPUT_CONTROL_POINT_COUNT:
    pInstruction->m_InputControlPointCountDecl.InputControlPointCount =
        DECODE_D3D11_SB_INPUT_CONTROL_POINT_COUNT(Token);
    break;

  case D3D11_SB_OPCODE_DCL_OUTPUT_CONTROL_POINT_COUNT:
    pInstruction->m_OutputControlPointCountDecl.OutputControlPointCount =
        DECODE_D3D11_SB_OUTPUT_CONTROL_POINT_COUNT(Token);
    break;

  case D3D11_SB_OPCODE_DCL_TESS_DOMAIN:
    pInstruction->m_TessellatorDomainDecl.TessellatorDomain =
        DECODE_D3D11_SB_TESS_DOMAIN(Token);
    break;

  case D3D11_SB_OPCODE_DCL_TESS_PARTITIONING:
    pInstruction->m_TessellatorPartitioningDecl.TessellatorPartitioning =
        DECODE_D3D11_SB_TESS_PARTITIONING(Token);
    break;

  case D3D11_SB_OPCODE_DCL_TESS_OUTPUT_PRIMITIVE:
    pInstruction->m_TessellatorOutputPrimitiveDecl.TessellatorOutputPrimitive =
        DECODE_D3D11_SB_TESS_OUTPUT_PRIMITIVE(Token);
    break;

  case D3D11_SB_OPCODE_DCL_HS_MAX_TESSFACTOR:
    pInstruction->m_HSMaxTessFactorDecl.MaxTessFactor =
        *(const float *)m_pCurrentToken;
    m_pCurrentToken++;
    break;

  case D3D11_SB_OPCODE_DCL_HS_FORK_PHASE_INSTANCE_COUNT:
    pInstruction->m_HSForkPhaseInstanceCountDecl.InstanceCount =
        *(const UINT *)m_pCurrentToken;
    m_pCurrentToken++;
    break;
  case D3D11_SB_OPCODE_DCL_HS_JOIN_PHASE_INSTANCE_COUNT:
    pInstruction->m_HSJoinPhaseInstanceCountDecl.InstanceCount =
        *(const UINT *)m_pCurrentToken;
    m_pCurrentToken++;
    break;
  case D3D11_SB_OPCODE_DCL_THREAD_GROUP:
    pInstruction->m_ThreadGroupDecl.x = *(const UINT *)m_pCurrentToken++;
    pInstruction->m_ThreadGroupDecl.y = *(const UINT *)m_pCurrentToken++;
    pInstruction->m_ThreadGroupDecl.z = *(const UINT *)m_pCurrentToken++;
    break;
  case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_TYPED:
    pInstruction->m_TypedUAVDecl.Dimension =
        DECODE_D3D10_SB_RESOURCE_DIMENSION(Token);
    pInstruction->m_TypedUAVDecl.Flags = DECODE_D3D11_SB_RESOURCE_FLAGS(Token);
    ParseOperand(&pInstruction->m_Operands[0]);
    pInstruction->m_TypedUAVDecl.ReturnType[0] =
        DECODE_D3D10_SB_RESOURCE_RETURN_TYPE(*m_pCurrentToken, 0);
    pInstruction->m_TypedUAVDecl.ReturnType[1] =
        DECODE_D3D10_SB_RESOURCE_RETURN_TYPE(*m_pCurrentToken, 1);
    pInstruction->m_TypedUAVDecl.ReturnType[2] =
        DECODE_D3D10_SB_RESOURCE_RETURN_TYPE(*m_pCurrentToken, 2);
    pInstruction->m_TypedUAVDecl.ReturnType[3] =
        DECODE_D3D10_SB_RESOURCE_RETURN_TYPE(*m_pCurrentToken, 3);
    m_pCurrentToken++;
    pInstruction->m_TypedUAVDecl.Space = 0;
    if (b51PlusShader) {
      pInstruction->m_TypedUAVDecl.Space = (UINT)(*m_pCurrentToken++);
    }
    break;
  case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_RAW:
    pInstruction->m_RawUAVDecl.Flags = DECODE_D3D11_SB_RESOURCE_FLAGS(Token);
    ParseOperand(&pInstruction->m_Operands[0]);
    pInstruction->m_RawUAVDecl.Space = 0;
    if (b51PlusShader) {
      pInstruction->m_RawUAVDecl.Space = (UINT)(*m_pCurrentToken++);
    }
    break;
  case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_STRUCTURED:
    pInstruction->m_StructuredUAVDecl.Flags =
        DECODE_D3D11_SB_RESOURCE_FLAGS(Token);
    ParseOperand(&pInstruction->m_Operands[0]);
    pInstruction->m_StructuredUAVDecl.ByteStride =
        *(const UINT *)m_pCurrentToken++;
    pInstruction->m_StructuredUAVDecl.Space = 0;
    if (b51PlusShader) {
      pInstruction->m_StructuredUAVDecl.Space = (UINT)(*m_pCurrentToken++);
    }
    break;
  case D3D11_SB_OPCODE_DCL_THREAD_GROUP_SHARED_MEMORY_RAW:
    ParseOperand(&pInstruction->m_Operands[0]);
    pInstruction->m_RawTGSMDecl.ByteCount = *(const UINT *)m_pCurrentToken++;
    break;
  case D3D11_SB_OPCODE_DCL_THREAD_GROUP_SHARED_MEMORY_STRUCTURED:
    ParseOperand(&pInstruction->m_Operands[0]);
    pInstruction->m_StructuredTGSMDecl.StructByteStride =
        *(const UINT *)m_pCurrentToken++;
    pInstruction->m_StructuredTGSMDecl.StructCount =
        *(const UINT *)m_pCurrentToken++;
    break;
  case D3D11_SB_OPCODE_DCL_RESOURCE_RAW:
    ParseOperand(&pInstruction->m_Operands[0]);
    pInstruction->m_RawSRVDecl.Space = 0;
    if (b51PlusShader) {
      pInstruction->m_RawSRVDecl.Space = (UINT)(*m_pCurrentToken++);
    }
    break;
  case D3D11_SB_OPCODE_DCL_RESOURCE_STRUCTURED:
    ParseOperand(&pInstruction->m_Operands[0]);
    pInstruction->m_StructuredSRVDecl.ByteStride =
        *(const UINT *)m_pCurrentToken++;
    pInstruction->m_StructuredSRVDecl.Space = 0;
    if (b51PlusShader) {
      pInstruction->m_StructuredSRVDecl.Space = (UINT)(*m_pCurrentToken++);
    }
    break;
  case D3D11_SB_OPCODE_SYNC: {
    DWORD flags = DECODE_D3D11_SB_SYNC_FLAGS(Token);
    pInstruction->m_SyncFlags.bThreadsInGroup =
        (flags & D3D11_SB_SYNC_THREADS_IN_GROUP) ? true : false;
    pInstruction->m_SyncFlags.bThreadGroupSharedMemory =
        (flags & D3D11_SB_SYNC_THREAD_GROUP_SHARED_MEMORY) ? true : false;
    pInstruction->m_SyncFlags.bUnorderedAccessViewMemoryGroup =
        (flags & D3D11_SB_SYNC_UNORDERED_ACCESS_VIEW_MEMORY_GROUP) ? true
                                                                   : false;
    pInstruction->m_SyncFlags.bUnorderedAccessViewMemoryGlobal =
        (flags & D3D11_SB_SYNC_UNORDERED_ACCESS_VIEW_MEMORY_GLOBAL) ? true
                                                                    : false;
  } break;
  case D3D10_SB_OPCODE_RESINFO:
    pInstruction->m_ResInfoReturnType =
        DECODE_D3D10_SB_RESINFO_INSTRUCTION_RETURN_TYPE(Token);
    ParseOperand(&pInstruction->m_Operands[0]);
    ParseOperand(&pInstruction->m_Operands[1]);
    ParseOperand(&pInstruction->m_Operands[2]);
    break;

  case D3D10_1_SB_OPCODE_SAMPLE_INFO:
    pInstruction->m_InstructionReturnType =
        DECODE_D3D10_SB_INSTRUCTION_RETURN_TYPE(Token);
    ParseOperand(&pInstruction->m_Operands[0]);
    ParseOperand(&pInstruction->m_Operands[1]);
    break;

  case D3D10_SB_OPCODE_IF:
  case D3D10_SB_OPCODE_BREAKC:
  case D3D10_SB_OPCODE_CONTINUEC:
  case D3D10_SB_OPCODE_RETC:
  case D3D10_SB_OPCODE_DISCARD:
    pInstruction->SetTest(DECODE_D3D10_SB_INSTRUCTION_TEST_BOOLEAN(Token));
    ParseOperand(&pInstruction->m_Operands[0]);
    break;
  case D3D10_SB_OPCODE_CALLC:
    pInstruction->SetTest(DECODE_D3D10_SB_INSTRUCTION_TEST_BOOLEAN(Token));
    ParseOperand(&pInstruction->m_Operands[0]);
    ParseOperand(&pInstruction->m_Operands[1]);
    break;
  default: {

    for (UINT i = 0; i < pInstruction->m_NumOperands; i++) {
      ParseOperand(&pInstruction->m_Operands[i]);
    }
    break;
  }
  }
  m_pCurrentToken = pStart + InstructionLength;
}

// ****************************************************************************
//
// class CShaderAsm
//
// ****************************************************************************

void CShaderAsm::EmitOperand(const COperandBase &operand) {
  CShaderToken Token =
      ENCODE_D3D10_SB_OPERAND_TYPE(operand.m_Type) |
      ENCODE_D3D10_SB_OPERAND_NUM_COMPONENTS(operand.m_NumComponents) |
      ENCODE_D3D10_SB_OPERAND_EXTENDED(operand.m_bExtendedOperand);

  BOOL bProcessOperandIndices = FALSE;
  if (!(operand.m_Type == D3D10_SB_OPERAND_TYPE_IMMEDIATE32 ||
        operand.m_Type == D3D10_SB_OPERAND_TYPE_IMMEDIATE64)) {
    Token |= ENCODE_D3D10_SB_OPERAND_INDEX_DIMENSION(operand.m_IndexDimension);
    if (operand.m_NumComponents == D3D10_SB_OPERAND_4_COMPONENT) {
      // Component selection mode
      Token |= ENCODE_D3D10_SB_OPERAND_4_COMPONENT_SELECTION_MODE(
          operand.m_ComponentSelection);
      switch (operand.m_ComponentSelection) {
      case D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE:
        Token |= ENCODE_D3D10_SB_OPERAND_4_COMPONENT_MASK(operand.m_WriteMask);
        break;
      case D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE:
        Token |= ENCODE_D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE(
            operand.m_Swizzle[0], operand.m_Swizzle[1], operand.m_Swizzle[2],
            operand.m_Swizzle[3]);
        break;
      case D3D10_SB_OPERAND_4_COMPONENT_SELECT_1_MODE: {
        Token |= ENCODE_D3D10_SB_OPERAND_4_COMPONENT_SELECT_1(
            operand.m_ComponentName);
        break;
      }
      default:
        throw E_FAIL;
      }
    }

    UINT NumDimensions = operand.m_IndexDimension;
    if (NumDimensions > 0) {
      bProcessOperandIndices = TRUE;
      // Encode index representation
      for (UINT i = 0; i < NumDimensions; i++) {
        Token |= ENCODE_D3D10_SB_OPERAND_INDEX_REPRESENTATION(
            i, operand.m_IndexType[i]);
      }
    }
    FUNC(Token);
  }

  // Extended operand
  if (operand.m_bExtendedOperand) {
    Token =
        ENCODE_D3D10_SB_EXTENDED_OPERAND_TYPE(operand.m_ExtendedOperandType);
    if (operand.m_ExtendedOperandType == D3D10_SB_EXTENDED_OPERAND_MODIFIER) {
      Token |= ENCODE_D3D10_SB_EXTENDED_OPERAND_MODIFIER(operand.m_Modifier);
      Token |= ENCODE_D3D11_SB_OPERAND_MIN_PRECISION(operand.m_MinPrecision);
      Token |= ENCODE_D3D12_SB_OPERAND_NON_UNIFORM(operand.m_Nonuniform);
    }
    FUNC(Token);
  }

  if (operand.m_Type == D3D10_SB_OPERAND_TYPE_IMMEDIATE32 ||
      operand.m_Type == D3D10_SB_OPERAND_TYPE_IMMEDIATE64) {
    FUNC(Token);
    UINT n = 0;
    if (operand.m_NumComponents == D3D10_SB_OPERAND_4_COMPONENT)
      n = 4;
    else if (operand.m_NumComponents == D3D10_SB_OPERAND_1_COMPONENT)
      n = 1;
    else {
      throw E_FAIL;
    }
    for (UINT i = 0; i < n; i++) {
      FUNC(operand.m_Value[i]);
    }
  }

  // Operand indices
  if (bProcessOperandIndices) {
    const UINT NumDimensions = operand.m_IndexDimension;
    // Encode index representation
    for (UINT i = 0; i < NumDimensions; i++) {
      switch (operand.m_IndexType[i]) {
      case D3D10_SB_OPERAND_INDEX_IMMEDIATE32:
        FUNC(operand.m_Index[i].m_RegIndex);
        break;
      case D3D10_SB_OPERAND_INDEX_IMMEDIATE64:
        FUNC(operand.m_Index[i].m_RegIndexA[0]);
        FUNC(operand.m_Index[i].m_RegIndexA[1]);
        break;
      case D3D10_SB_OPERAND_INDEX_IMMEDIATE32_PLUS_RELATIVE:
        FUNC(operand.m_Index[i].m_RegIndex);
        // Fall through
        LLVM_FALLTHROUGH;
      case D3D10_SB_OPERAND_INDEX_RELATIVE: {
        D3D10_SB_OPERAND_TYPE RelRegType = operand.m_Index[i].m_RelRegType;
        if (operand.m_Index[i].m_IndexDimension == D3D10_SB_OPERAND_INDEX_2D) {
          EmitOperand(COperand2D(RelRegType, operand.m_Index[i].m_RelIndex,
                                 operand.m_Index[i].m_RelIndex1,
                                 operand.m_Index[i].m_ComponentName,
                                 operand.m_Index[i].m_MinPrecision));
        } else {
          EmitOperand(COperand4(RelRegType, operand.m_Index[i].m_RelIndex,
                                operand.m_Index[i].m_ComponentName,
                                operand.m_Index[i].m_MinPrecision));
        }
      } break;
      default:
        throw E_FAIL;
      }
    }
  }
}
//-----------------------------------------------------------------------------
void CShaderAsm::EmitInstruction(const CInstruction &instruction) {
  UINT OpCode;

  if (instruction.m_OpCode == D3D10_SB_OPCODE_CUSTOMDATA) {
    OPCODE(D3D10_SB_OPCODE_CUSTOMDATA);
    FUNC(instruction.m_CustomData.DataSizeInBytes / 4 + 2);
    for (UINT i = 0; i < instruction.m_CustomData.DataSizeInBytes / 4; i++)
      FUNC(((UINT *)instruction.m_CustomData.pData)[i]);

    ENDINSTRUCTION();
    return;
  }

  OpCode = ENCODE_D3D10_SB_OPCODE_TYPE(instruction.m_OpCode) |
           ENCODE_D3D10_SB_OPCODE_EXTENDED(
               instruction.m_ExtendedOpCodeCount > 0 ? true : false);
  switch (instruction.m_OpCode) {
  case D3D10_SB_OPCODE_IF:
  case D3D10_SB_OPCODE_BREAKC:
  case D3D10_SB_OPCODE_CALLC:
  case D3D10_SB_OPCODE_CONTINUEC:
  case D3D10_SB_OPCODE_RETC:
  case D3D10_SB_OPCODE_DISCARD:
    OpCode |= ENCODE_D3D10_SB_INSTRUCTION_TEST_BOOLEAN(instruction.Test());
    break;
  case D3D10_SB_OPCODE_RESINFO:
    OpCode |= ENCODE_D3D10_SB_RESINFO_INSTRUCTION_RETURN_TYPE(
        instruction.m_ResInfoReturnType);
    break;
  case D3D10_1_SB_OPCODE_SAMPLE_INFO:
    OpCode |= ENCODE_D3D10_SB_INSTRUCTION_RETURN_TYPE(
        instruction.m_InstructionReturnType);
    break;
  case D3D11_SB_OPCODE_SYNC:
    OpCode |= ENCODE_D3D11_SB_SYNC_FLAGS(
        (instruction.m_SyncFlags.bThreadsInGroup
             ? D3D11_SB_SYNC_THREADS_IN_GROUP
             : 0) |
        (instruction.m_SyncFlags.bThreadGroupSharedMemory
             ? D3D11_SB_SYNC_THREAD_GROUP_SHARED_MEMORY
             : 0) |
        (instruction.m_SyncFlags.bUnorderedAccessViewMemoryGlobal
             ? D3D11_SB_SYNC_UNORDERED_ACCESS_VIEW_MEMORY_GLOBAL
             : 0) |
        (instruction.m_SyncFlags.bUnorderedAccessViewMemoryGroup
             ? D3D11_SB_SYNC_UNORDERED_ACCESS_VIEW_MEMORY_GROUP
             : 0));
    break;
  };
  OpCode |= ENCODE_D3D10_SB_INSTRUCTION_SATURATE(instruction.m_bSaturate);
  OpCode |=
      ENCODE_D3D11_SB_INSTRUCTION_PRECISE_VALUES(instruction.m_PreciseMask);
  OPCODE(OpCode);

  for (UINT i = 0; i < min(instruction.m_ExtendedOpCodeCount,
                           (UINT)D3D11_SB_MAX_SIMULTANEOUS_EXTENDED_OPCODES);
       i++) {
    UINT Extended =
        ENCODE_D3D10_SB_EXTENDED_OPCODE_TYPE(instruction.m_OpCodeEx[i]);
    switch (instruction.m_OpCodeEx[i]) {
    case D3D10_SB_EXTENDED_OPCODE_SAMPLE_CONTROLS: {
      Extended |= ENCODE_IMMEDIATE_D3D10_SB_ADDRESS_OFFSET(
          D3D10_SB_IMMEDIATE_ADDRESS_OFFSET_U, instruction.m_TexelOffset[0]);
      Extended |= ENCODE_IMMEDIATE_D3D10_SB_ADDRESS_OFFSET(
          D3D10_SB_IMMEDIATE_ADDRESS_OFFSET_V, instruction.m_TexelOffset[1]);
      Extended |= ENCODE_IMMEDIATE_D3D10_SB_ADDRESS_OFFSET(
          D3D10_SB_IMMEDIATE_ADDRESS_OFFSET_W, instruction.m_TexelOffset[2]);
    } break;
    case D3D11_SB_EXTENDED_OPCODE_RESOURCE_DIM: {
      Extended |= ENCODE_D3D11_SB_EXTENDED_RESOURCE_DIMENSION(
                      instruction.m_ResourceDimEx) |
                  ENCODE_D3D11_SB_EXTENDED_RESOURCE_DIMENSION_STRUCTURE_STRIDE(
                      instruction.m_ResourceDimStructureStrideEx);
    } break;
    case D3D11_SB_EXTENDED_OPCODE_RESOURCE_RETURN_TYPE: {
      for (UINT j = 0; j < 4; j++) {
        Extended |= ENCODE_D3D11_SB_EXTENDED_RESOURCE_RETURN_TYPE(
            instruction.m_ResourceReturnTypeEx[j], j);
      }
    } break;
    }
    Extended |= ENCODE_D3D10_SB_OPCODE_EXTENDED(
        (i + 1 < instruction.m_ExtendedOpCodeCount) ? true : false);
    FUNC(Extended);
  }
  for (UINT i = 0; i < instruction.m_NumOperands; i++) {
    EmitOperand(instruction.m_Operands[i]);
  }
  ENDINSTRUCTION();
}

//*****************************************************************************
//
//  CInstruction
//
//*****************************************************************************
BOOL CInstruction::Disassemble(__out_ecount(StringSize) LPSTR pString,
                               UINT StringSize) {
  StringCchCopyA(pString, StringSize, g_InstructionInfo[m_OpCode].m_Name);
  return TRUE;
}

}; // namespace D3D10ShaderBinary

// End of file : ShaderBinary.cpp
