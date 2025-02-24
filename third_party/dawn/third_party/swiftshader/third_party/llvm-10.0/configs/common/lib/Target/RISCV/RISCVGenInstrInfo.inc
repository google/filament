/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Target Instruction Enum Values and Descriptors                             *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifdef GET_INSTRINFO_ENUM
#undef GET_INSTRINFO_ENUM
namespace llvm {

namespace RISCV {
  enum {
    PHI	= 0,
    INLINEASM	= 1,
    INLINEASM_BR	= 2,
    CFI_INSTRUCTION	= 3,
    EH_LABEL	= 4,
    GC_LABEL	= 5,
    ANNOTATION_LABEL	= 6,
    KILL	= 7,
    EXTRACT_SUBREG	= 8,
    INSERT_SUBREG	= 9,
    IMPLICIT_DEF	= 10,
    SUBREG_TO_REG	= 11,
    COPY_TO_REGCLASS	= 12,
    DBG_VALUE	= 13,
    DBG_LABEL	= 14,
    REG_SEQUENCE	= 15,
    COPY	= 16,
    BUNDLE	= 17,
    LIFETIME_START	= 18,
    LIFETIME_END	= 19,
    STACKMAP	= 20,
    FENTRY_CALL	= 21,
    PATCHPOINT	= 22,
    LOAD_STACK_GUARD	= 23,
    STATEPOINT	= 24,
    LOCAL_ESCAPE	= 25,
    FAULTING_OP	= 26,
    PATCHABLE_OP	= 27,
    PATCHABLE_FUNCTION_ENTER	= 28,
    PATCHABLE_RET	= 29,
    PATCHABLE_FUNCTION_EXIT	= 30,
    PATCHABLE_TAIL_CALL	= 31,
    PATCHABLE_EVENT_CALL	= 32,
    PATCHABLE_TYPED_EVENT_CALL	= 33,
    ICALL_BRANCH_FUNNEL	= 34,
    G_ADD	= 35,
    G_SUB	= 36,
    G_MUL	= 37,
    G_SDIV	= 38,
    G_UDIV	= 39,
    G_SREM	= 40,
    G_UREM	= 41,
    G_AND	= 42,
    G_OR	= 43,
    G_XOR	= 44,
    G_IMPLICIT_DEF	= 45,
    G_PHI	= 46,
    G_FRAME_INDEX	= 47,
    G_GLOBAL_VALUE	= 48,
    G_EXTRACT	= 49,
    G_UNMERGE_VALUES	= 50,
    G_INSERT	= 51,
    G_MERGE_VALUES	= 52,
    G_BUILD_VECTOR	= 53,
    G_BUILD_VECTOR_TRUNC	= 54,
    G_CONCAT_VECTORS	= 55,
    G_PTRTOINT	= 56,
    G_INTTOPTR	= 57,
    G_BITCAST	= 58,
    G_INTRINSIC_TRUNC	= 59,
    G_INTRINSIC_ROUND	= 60,
    G_READCYCLECOUNTER	= 61,
    G_LOAD	= 62,
    G_SEXTLOAD	= 63,
    G_ZEXTLOAD	= 64,
    G_INDEXED_LOAD	= 65,
    G_INDEXED_SEXTLOAD	= 66,
    G_INDEXED_ZEXTLOAD	= 67,
    G_STORE	= 68,
    G_INDEXED_STORE	= 69,
    G_ATOMIC_CMPXCHG_WITH_SUCCESS	= 70,
    G_ATOMIC_CMPXCHG	= 71,
    G_ATOMICRMW_XCHG	= 72,
    G_ATOMICRMW_ADD	= 73,
    G_ATOMICRMW_SUB	= 74,
    G_ATOMICRMW_AND	= 75,
    G_ATOMICRMW_NAND	= 76,
    G_ATOMICRMW_OR	= 77,
    G_ATOMICRMW_XOR	= 78,
    G_ATOMICRMW_MAX	= 79,
    G_ATOMICRMW_MIN	= 80,
    G_ATOMICRMW_UMAX	= 81,
    G_ATOMICRMW_UMIN	= 82,
    G_ATOMICRMW_FADD	= 83,
    G_ATOMICRMW_FSUB	= 84,
    G_FENCE	= 85,
    G_BRCOND	= 86,
    G_BRINDIRECT	= 87,
    G_INTRINSIC	= 88,
    G_INTRINSIC_W_SIDE_EFFECTS	= 89,
    G_ANYEXT	= 90,
    G_TRUNC	= 91,
    G_CONSTANT	= 92,
    G_FCONSTANT	= 93,
    G_VASTART	= 94,
    G_VAARG	= 95,
    G_SEXT	= 96,
    G_SEXT_INREG	= 97,
    G_ZEXT	= 98,
    G_SHL	= 99,
    G_LSHR	= 100,
    G_ASHR	= 101,
    G_ICMP	= 102,
    G_FCMP	= 103,
    G_SELECT	= 104,
    G_UADDO	= 105,
    G_UADDE	= 106,
    G_USUBO	= 107,
    G_USUBE	= 108,
    G_SADDO	= 109,
    G_SADDE	= 110,
    G_SSUBO	= 111,
    G_SSUBE	= 112,
    G_UMULO	= 113,
    G_SMULO	= 114,
    G_UMULH	= 115,
    G_SMULH	= 116,
    G_FADD	= 117,
    G_FSUB	= 118,
    G_FMUL	= 119,
    G_FMA	= 120,
    G_FMAD	= 121,
    G_FDIV	= 122,
    G_FREM	= 123,
    G_FPOW	= 124,
    G_FEXP	= 125,
    G_FEXP2	= 126,
    G_FLOG	= 127,
    G_FLOG2	= 128,
    G_FLOG10	= 129,
    G_FNEG	= 130,
    G_FPEXT	= 131,
    G_FPTRUNC	= 132,
    G_FPTOSI	= 133,
    G_FPTOUI	= 134,
    G_SITOFP	= 135,
    G_UITOFP	= 136,
    G_FABS	= 137,
    G_FCOPYSIGN	= 138,
    G_FCANONICALIZE	= 139,
    G_FMINNUM	= 140,
    G_FMAXNUM	= 141,
    G_FMINNUM_IEEE	= 142,
    G_FMAXNUM_IEEE	= 143,
    G_FMINIMUM	= 144,
    G_FMAXIMUM	= 145,
    G_PTR_ADD	= 146,
    G_PTR_MASK	= 147,
    G_SMIN	= 148,
    G_SMAX	= 149,
    G_UMIN	= 150,
    G_UMAX	= 151,
    G_BR	= 152,
    G_BRJT	= 153,
    G_INSERT_VECTOR_ELT	= 154,
    G_EXTRACT_VECTOR_ELT	= 155,
    G_SHUFFLE_VECTOR	= 156,
    G_CTTZ	= 157,
    G_CTTZ_ZERO_UNDEF	= 158,
    G_CTLZ	= 159,
    G_CTLZ_ZERO_UNDEF	= 160,
    G_CTPOP	= 161,
    G_BSWAP	= 162,
    G_BITREVERSE	= 163,
    G_FCEIL	= 164,
    G_FCOS	= 165,
    G_FSIN	= 166,
    G_FSQRT	= 167,
    G_FFLOOR	= 168,
    G_FRINT	= 169,
    G_FNEARBYINT	= 170,
    G_ADDRSPACE_CAST	= 171,
    G_BLOCK_ADDR	= 172,
    G_JUMP_TABLE	= 173,
    G_DYN_STACKALLOC	= 174,
    G_READ_REGISTER	= 175,
    G_WRITE_REGISTER	= 176,
    ADJCALLSTACKDOWN	= 177,
    ADJCALLSTACKUP	= 178,
    BuildPairF64Pseudo	= 179,
    PseudoAddTPRel	= 180,
    PseudoAtomicLoadNand32	= 181,
    PseudoAtomicLoadNand64	= 182,
    PseudoBR	= 183,
    PseudoBRIND	= 184,
    PseudoCALL	= 185,
    PseudoCALLIndirect	= 186,
    PseudoCALLReg	= 187,
    PseudoCmpXchg32	= 188,
    PseudoCmpXchg64	= 189,
    PseudoFLD	= 190,
    PseudoFLW	= 191,
    PseudoFSD	= 192,
    PseudoFSW	= 193,
    PseudoLA	= 194,
    PseudoLA_TLS_GD	= 195,
    PseudoLA_TLS_IE	= 196,
    PseudoLB	= 197,
    PseudoLBU	= 198,
    PseudoLD	= 199,
    PseudoLH	= 200,
    PseudoLHU	= 201,
    PseudoLI	= 202,
    PseudoLLA	= 203,
    PseudoLW	= 204,
    PseudoLWU	= 205,
    PseudoMaskedAtomicLoadAdd32	= 206,
    PseudoMaskedAtomicLoadMax32	= 207,
    PseudoMaskedAtomicLoadMin32	= 208,
    PseudoMaskedAtomicLoadNand32	= 209,
    PseudoMaskedAtomicLoadSub32	= 210,
    PseudoMaskedAtomicLoadUMax32	= 211,
    PseudoMaskedAtomicLoadUMin32	= 212,
    PseudoMaskedAtomicSwap32	= 213,
    PseudoMaskedCmpXchg32	= 214,
    PseudoRET	= 215,
    PseudoSB	= 216,
    PseudoSD	= 217,
    PseudoSH	= 218,
    PseudoSW	= 219,
    PseudoTAIL	= 220,
    PseudoTAILIndirect	= 221,
    ReadCycleWide	= 222,
    Select_FPR32_Using_CC_GPR	= 223,
    Select_FPR64_Using_CC_GPR	= 224,
    Select_GPR_Using_CC_GPR	= 225,
    SplitF64Pseudo	= 226,
    ADD	= 227,
    ADDI	= 228,
    ADDIW	= 229,
    ADDW	= 230,
    AMOADD_D	= 231,
    AMOADD_D_AQ	= 232,
    AMOADD_D_AQ_RL	= 233,
    AMOADD_D_RL	= 234,
    AMOADD_W	= 235,
    AMOADD_W_AQ	= 236,
    AMOADD_W_AQ_RL	= 237,
    AMOADD_W_RL	= 238,
    AMOAND_D	= 239,
    AMOAND_D_AQ	= 240,
    AMOAND_D_AQ_RL	= 241,
    AMOAND_D_RL	= 242,
    AMOAND_W	= 243,
    AMOAND_W_AQ	= 244,
    AMOAND_W_AQ_RL	= 245,
    AMOAND_W_RL	= 246,
    AMOMAXU_D	= 247,
    AMOMAXU_D_AQ	= 248,
    AMOMAXU_D_AQ_RL	= 249,
    AMOMAXU_D_RL	= 250,
    AMOMAXU_W	= 251,
    AMOMAXU_W_AQ	= 252,
    AMOMAXU_W_AQ_RL	= 253,
    AMOMAXU_W_RL	= 254,
    AMOMAX_D	= 255,
    AMOMAX_D_AQ	= 256,
    AMOMAX_D_AQ_RL	= 257,
    AMOMAX_D_RL	= 258,
    AMOMAX_W	= 259,
    AMOMAX_W_AQ	= 260,
    AMOMAX_W_AQ_RL	= 261,
    AMOMAX_W_RL	= 262,
    AMOMINU_D	= 263,
    AMOMINU_D_AQ	= 264,
    AMOMINU_D_AQ_RL	= 265,
    AMOMINU_D_RL	= 266,
    AMOMINU_W	= 267,
    AMOMINU_W_AQ	= 268,
    AMOMINU_W_AQ_RL	= 269,
    AMOMINU_W_RL	= 270,
    AMOMIN_D	= 271,
    AMOMIN_D_AQ	= 272,
    AMOMIN_D_AQ_RL	= 273,
    AMOMIN_D_RL	= 274,
    AMOMIN_W	= 275,
    AMOMIN_W_AQ	= 276,
    AMOMIN_W_AQ_RL	= 277,
    AMOMIN_W_RL	= 278,
    AMOOR_D	= 279,
    AMOOR_D_AQ	= 280,
    AMOOR_D_AQ_RL	= 281,
    AMOOR_D_RL	= 282,
    AMOOR_W	= 283,
    AMOOR_W_AQ	= 284,
    AMOOR_W_AQ_RL	= 285,
    AMOOR_W_RL	= 286,
    AMOSWAP_D	= 287,
    AMOSWAP_D_AQ	= 288,
    AMOSWAP_D_AQ_RL	= 289,
    AMOSWAP_D_RL	= 290,
    AMOSWAP_W	= 291,
    AMOSWAP_W_AQ	= 292,
    AMOSWAP_W_AQ_RL	= 293,
    AMOSWAP_W_RL	= 294,
    AMOXOR_D	= 295,
    AMOXOR_D_AQ	= 296,
    AMOXOR_D_AQ_RL	= 297,
    AMOXOR_D_RL	= 298,
    AMOXOR_W	= 299,
    AMOXOR_W_AQ	= 300,
    AMOXOR_W_AQ_RL	= 301,
    AMOXOR_W_RL	= 302,
    AND	= 303,
    ANDI	= 304,
    AUIPC	= 305,
    BEQ	= 306,
    BGE	= 307,
    BGEU	= 308,
    BLT	= 309,
    BLTU	= 310,
    BNE	= 311,
    CSRRC	= 312,
    CSRRCI	= 313,
    CSRRS	= 314,
    CSRRSI	= 315,
    CSRRW	= 316,
    CSRRWI	= 317,
    C_ADD	= 318,
    C_ADDI	= 319,
    C_ADDI16SP	= 320,
    C_ADDI4SPN	= 321,
    C_ADDIW	= 322,
    C_ADDI_HINT_IMM_ZERO	= 323,
    C_ADDI_HINT_X0	= 324,
    C_ADDI_NOP	= 325,
    C_ADDW	= 326,
    C_ADD_HINT	= 327,
    C_AND	= 328,
    C_ANDI	= 329,
    C_BEQZ	= 330,
    C_BNEZ	= 331,
    C_EBREAK	= 332,
    C_FLD	= 333,
    C_FLDSP	= 334,
    C_FLW	= 335,
    C_FLWSP	= 336,
    C_FSD	= 337,
    C_FSDSP	= 338,
    C_FSW	= 339,
    C_FSWSP	= 340,
    C_J	= 341,
    C_JAL	= 342,
    C_JALR	= 343,
    C_JR	= 344,
    C_LD	= 345,
    C_LDSP	= 346,
    C_LI	= 347,
    C_LI_HINT	= 348,
    C_LUI	= 349,
    C_LUI_HINT	= 350,
    C_LW	= 351,
    C_LWSP	= 352,
    C_MV	= 353,
    C_MV_HINT	= 354,
    C_NOP	= 355,
    C_NOP_HINT	= 356,
    C_OR	= 357,
    C_SD	= 358,
    C_SDSP	= 359,
    C_SLLI	= 360,
    C_SLLI64_HINT	= 361,
    C_SLLI_HINT	= 362,
    C_SRAI	= 363,
    C_SRAI64_HINT	= 364,
    C_SRLI	= 365,
    C_SRLI64_HINT	= 366,
    C_SUB	= 367,
    C_SUBW	= 368,
    C_SW	= 369,
    C_SWSP	= 370,
    C_UNIMP	= 371,
    C_XOR	= 372,
    DIV	= 373,
    DIVU	= 374,
    DIVUW	= 375,
    DIVW	= 376,
    EBREAK	= 377,
    ECALL	= 378,
    FADD_D	= 379,
    FADD_S	= 380,
    FCLASS_D	= 381,
    FCLASS_S	= 382,
    FCVT_D_L	= 383,
    FCVT_D_LU	= 384,
    FCVT_D_S	= 385,
    FCVT_D_W	= 386,
    FCVT_D_WU	= 387,
    FCVT_LU_D	= 388,
    FCVT_LU_S	= 389,
    FCVT_L_D	= 390,
    FCVT_L_S	= 391,
    FCVT_S_D	= 392,
    FCVT_S_L	= 393,
    FCVT_S_LU	= 394,
    FCVT_S_W	= 395,
    FCVT_S_WU	= 396,
    FCVT_WU_D	= 397,
    FCVT_WU_S	= 398,
    FCVT_W_D	= 399,
    FCVT_W_S	= 400,
    FDIV_D	= 401,
    FDIV_S	= 402,
    FENCE	= 403,
    FENCE_I	= 404,
    FENCE_TSO	= 405,
    FEQ_D	= 406,
    FEQ_S	= 407,
    FLD	= 408,
    FLE_D	= 409,
    FLE_S	= 410,
    FLT_D	= 411,
    FLT_S	= 412,
    FLW	= 413,
    FMADD_D	= 414,
    FMADD_S	= 415,
    FMAX_D	= 416,
    FMAX_S	= 417,
    FMIN_D	= 418,
    FMIN_S	= 419,
    FMSUB_D	= 420,
    FMSUB_S	= 421,
    FMUL_D	= 422,
    FMUL_S	= 423,
    FMV_D_X	= 424,
    FMV_W_X	= 425,
    FMV_X_D	= 426,
    FMV_X_W	= 427,
    FNMADD_D	= 428,
    FNMADD_S	= 429,
    FNMSUB_D	= 430,
    FNMSUB_S	= 431,
    FSD	= 432,
    FSGNJN_D	= 433,
    FSGNJN_S	= 434,
    FSGNJX_D	= 435,
    FSGNJX_S	= 436,
    FSGNJ_D	= 437,
    FSGNJ_S	= 438,
    FSQRT_D	= 439,
    FSQRT_S	= 440,
    FSUB_D	= 441,
    FSUB_S	= 442,
    FSW	= 443,
    JAL	= 444,
    JALR	= 445,
    LB	= 446,
    LBU	= 447,
    LD	= 448,
    LH	= 449,
    LHU	= 450,
    LR_D	= 451,
    LR_D_AQ	= 452,
    LR_D_AQ_RL	= 453,
    LR_D_RL	= 454,
    LR_W	= 455,
    LR_W_AQ	= 456,
    LR_W_AQ_RL	= 457,
    LR_W_RL	= 458,
    LUI	= 459,
    LW	= 460,
    LWU	= 461,
    MRET	= 462,
    MUL	= 463,
    MULH	= 464,
    MULHSU	= 465,
    MULHU	= 466,
    MULW	= 467,
    OR	= 468,
    ORI	= 469,
    REM	= 470,
    REMU	= 471,
    REMUW	= 472,
    REMW	= 473,
    SB	= 474,
    SC_D	= 475,
    SC_D_AQ	= 476,
    SC_D_AQ_RL	= 477,
    SC_D_RL	= 478,
    SC_W	= 479,
    SC_W_AQ	= 480,
    SC_W_AQ_RL	= 481,
    SC_W_RL	= 482,
    SD	= 483,
    SFENCE_VMA	= 484,
    SH	= 485,
    SLL	= 486,
    SLLI	= 487,
    SLLIW	= 488,
    SLLW	= 489,
    SLT	= 490,
    SLTI	= 491,
    SLTIU	= 492,
    SLTU	= 493,
    SRA	= 494,
    SRAI	= 495,
    SRAIW	= 496,
    SRAW	= 497,
    SRET	= 498,
    SRL	= 499,
    SRLI	= 500,
    SRLIW	= 501,
    SRLW	= 502,
    SUB	= 503,
    SUBW	= 504,
    SW	= 505,
    UNIMP	= 506,
    URET	= 507,
    WFI	= 508,
    XOR	= 509,
    XORI	= 510,
    INSTRUCTION_LIST_END = 511
  };

} // end namespace RISCV
} // end namespace llvm
#endif // GET_INSTRINFO_ENUM

#ifdef GET_INSTRINFO_SCHED_ENUM
#undef GET_INSTRINFO_SCHED_ENUM
namespace llvm {

namespace RISCV {
namespace Sched {
  enum {
    NoInstrModel	= 0,
    WriteIALU_ReadIALU_ReadIALU	= 1,
    WriteIALU_ReadIALU	= 2,
    WriteIALU32_ReadIALU32	= 3,
    WriteIALU32_ReadIALU32_ReadIALU32	= 4,
    WriteAtomicD_ReadAtomicDA_ReadAtomicDD	= 5,
    WriteAtomicW_ReadAtomicWA_ReadAtomicWD	= 6,
    WriteIALU	= 7,
    WriteJmp	= 8,
    WriteCSR_ReadCSR	= 9,
    WriteCSR	= 10,
    WriteFLD64_ReadMemBase	= 11,
    WriteFLD32_ReadMemBase	= 12,
    WriteFST64_ReadStoreData_ReadMemBase	= 13,
    WriteFST32_ReadStoreData_ReadMemBase	= 14,
    WriteJal	= 15,
    WriteJalr_ReadJalr	= 16,
    WriteJmpReg	= 17,
    WriteLDD_ReadMemBase	= 18,
    WriteLDW_ReadMemBase	= 19,
    WriteNop	= 20,
    WriteSTD_ReadStoreData_ReadMemBase	= 21,
    WriteShift_ReadShift	= 22,
    WriteSTW_ReadStoreData_ReadMemBase	= 23,
    WriteIDiv_ReadIDiv_ReadIDiv	= 24,
    WriteIDiv32_ReadIDiv32_ReadIDiv32	= 25,
    WriteFALU64_ReadFALU64_ReadFALU64	= 26,
    WriteFALU32_ReadFALU32_ReadFALU32	= 27,
    WriteFClass64_ReadFClass64	= 28,
    WriteFClass32_ReadFClass32	= 29,
    WriteFCvtI64ToF64_ReadFCvtI64ToF64	= 30,
    WriteFCvtF32ToF64_ReadFCvtF32ToF64	= 31,
    WriteFCvtI32ToF64_ReadFCvtI32ToF64	= 32,
    WriteFCvtF64ToI64_ReadFCvtF64ToI64	= 33,
    WriteFCvtF32ToI64_ReadFCvtF32ToI64	= 34,
    WriteFCvtF64ToF32_ReadFCvtF64ToF32	= 35,
    WriteFCvtI64ToF32_ReadFCvtI64ToF32	= 36,
    WriteFCvtI32ToF32_ReadFCvtI32ToF32	= 37,
    WriteFCvtF64ToI32_ReadFCvtF64ToI32	= 38,
    WriteFCvtF32ToI32_ReadFCvtF32ToI32	= 39,
    WriteFDiv32_ReadFDiv32_ReadFDiv32	= 40,
    WriteFCmp64_ReadFCmp64_ReadFCmp64	= 41,
    WriteFCmp32_ReadFCmp32_ReadFCmp32	= 42,
    WriteFMulAdd64_ReadFMulAdd64_ReadFMulAdd64_ReadFMulAdd64	= 43,
    WriteFMulAdd32_ReadFMulAdd32_ReadFMulAdd32_ReadFMulAdd32	= 44,
    WriteFMulSub64_ReadFMulSub64_ReadFMulSub64_ReadFMulSub64	= 45,
    WriteFMulSub32_ReadFMulSub32_ReadFMulSub32_ReadFMulSub32	= 46,
    WriteFMul32_ReadFMul32_ReadFMul32	= 47,
    WriteFMovI64ToF64_ReadFMovI64ToF64	= 48,
    WriteFMovI32ToF32_ReadFMovI32ToF32	= 49,
    WriteFMovF64ToI64_ReadFMovF64ToI64	= 50,
    WriteFMovF32ToI32_ReadFMovF32ToI32	= 51,
    WriteFSqrt32_ReadFSqrt32	= 52,
    WriteLDB_ReadMemBase	= 53,
    WriteLDH_ReadMemBase	= 54,
    WriteAtomicLDD_ReadAtomicLDD	= 55,
    WriteAtomicLDW_ReadAtomicLDW	= 56,
    WriteLDWU_ReadMemBase	= 57,
    WriteIMul_ReadIMul_ReadIMul	= 58,
    WriteIMul32_ReadIMul32_ReadIMul32	= 59,
    WriteSTB_ReadStoreData_ReadMemBase	= 60,
    WriteAtomicSTD_ReadAtomicSTD_ReadAtomicSTD	= 61,
    WriteAtomicSTW_ReadAtomicSTW_ReadAtomicSTW	= 62,
    WriteSTH_ReadStoreData_ReadMemBase	= 63,
    WriteShift32_ReadShift32	= 64,
    COPY	= 65,
    SCHED_LIST_END = 66
  };
} // end namespace Sched
} // end namespace RISCV
} // end namespace llvm
#endif // GET_INSTRINFO_SCHED_ENUM

#ifdef GET_INSTRINFO_MC_DESC
#undef GET_INSTRINFO_MC_DESC
namespace llvm {

static const MCPhysReg ImplicitList1[] = { RISCV::X2, 0 };
static const MCPhysReg ImplicitList2[] = { RISCV::X1, 0 };

static const MCOperandInfo OperandInfo2[] = { { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo3[] = { { -1, 0, MCOI::OPERAND_IMMEDIATE, 0 }, };
static const MCOperandInfo OperandInfo4[] = { { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_IMMEDIATE, 0 }, };
static const MCOperandInfo OperandInfo5[] = { { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, ((0 << 16) | (1 << MCOI::TIED_TO)) }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_IMMEDIATE, 0 }, };
static const MCOperandInfo OperandInfo6[] = { { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_IMMEDIATE, 0 }, };
static const MCOperandInfo OperandInfo7[] = { { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo8[] = { { -1, 0, MCOI::OPERAND_IMMEDIATE, 0 }, { -1, 0, MCOI::OPERAND_IMMEDIATE, 0 }, };
static const MCOperandInfo OperandInfo9[] = { { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_IMMEDIATE, 0 }, { -1, 0, MCOI::OPERAND_IMMEDIATE, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_IMMEDIATE, 0 }, { -1, 0, MCOI::OPERAND_IMMEDIATE, 0 }, };
static const MCOperandInfo OperandInfo10[] = { { 0, 0|(1<<MCOI::LookupPtrRegClass), MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo11[] = { { 0, 0|(1<<MCOI::LookupPtrRegClass), MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_IMMEDIATE, 0 }, };
static const MCOperandInfo OperandInfo12[] = { { 0, 0|(1<<MCOI::LookupPtrRegClass), MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo13[] = { { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { 0, 0|(1<<MCOI::LookupPtrRegClass), MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo14[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, };
static const MCOperandInfo OperandInfo15[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, };
static const MCOperandInfo OperandInfo16[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo17[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo18[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, };
static const MCOperandInfo OperandInfo19[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo20[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, };
static const MCOperandInfo OperandInfo21[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_2, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo22[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_2, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo23[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_2, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, };
static const MCOperandInfo OperandInfo24[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, };
static const MCOperandInfo OperandInfo25[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, };
static const MCOperandInfo OperandInfo26[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_IMM_0, 0 }, };
static const MCOperandInfo OperandInfo27[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, };
static const MCOperandInfo OperandInfo28[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, };
static const MCOperandInfo OperandInfo29[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, };
static const MCOperandInfo OperandInfo30[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, };
static const MCOperandInfo OperandInfo31[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo32[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, };
static const MCOperandInfo OperandInfo33[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_2, 0 }, };
static const MCOperandInfo OperandInfo34[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_2, 0 }, };
static const MCOperandInfo OperandInfo35[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo36[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_IMMEDIATE, 0 }, };
static const MCOperandInfo OperandInfo37[] = { { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, };
static const MCOperandInfo OperandInfo38[] = { { RISCV::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo39[] = { { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo40[] = { { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, (1 << MCOI::EARLY_CLOBBER) }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, (1 << MCOI::EARLY_CLOBBER) }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo41[] = { { -1, 0, RISCVOp::OPERAND_SIMM21_LSB0, 0 }, };
static const MCOperandInfo OperandInfo42[] = { { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, RISCVOp::OPERAND_SIMM12, 0 }, };
static const MCOperandInfo OperandInfo43[] = { { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo44[] = { { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo45[] = { { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, (1 << MCOI::EARLY_CLOBBER) }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, (1 << MCOI::EARLY_CLOBBER) }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo46[] = { { RISCV::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo47[] = { { RISCV::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo48[] = { { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, (1 << MCOI::EARLY_CLOBBER) }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, (1 << MCOI::EARLY_CLOBBER) }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, (1 << MCOI::EARLY_CLOBBER) }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo49[] = { { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, (1 << MCOI::EARLY_CLOBBER) }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, (1 << MCOI::EARLY_CLOBBER) }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, (1 << MCOI::EARLY_CLOBBER) }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo50[] = { { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, (1 << MCOI::EARLY_CLOBBER) }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, (1 << MCOI::EARLY_CLOBBER) }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo51[] = { { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo52[] = { { RISCV::GPRTCRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo53[] = { { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo54[] = { { RISCV::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { RISCV::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo55[] = { { RISCV::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { RISCV::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo56[] = { { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo57[] = { { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo58[] = { { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo59[] = { { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, RISCVOp::OPERAND_SIMM12, 0 }, };
static const MCOperandInfo OperandInfo60[] = { { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, RISCVOp::OPERAND_UIMM20, 0 }, };
static const MCOperandInfo OperandInfo61[] = { { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, RISCVOp::OPERAND_SIMM13_LSB0, 0 }, };
static const MCOperandInfo OperandInfo62[] = { { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, RISCVOp::OPERAND_UIMM12, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo63[] = { { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, RISCVOp::OPERAND_UIMM12, 0 }, { -1, 0, RISCVOp::OPERAND_UIMM5, 0 }, };
static const MCOperandInfo OperandInfo64[] = { { RISCV::GPRNoX0RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRNoX0RegClassID, 0, MCOI::OPERAND_REGISTER, ((0 << 16) | (1 << MCOI::TIED_TO)) }, { RISCV::GPRNoX0RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo65[] = { { RISCV::GPRNoX0RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRNoX0RegClassID, 0, MCOI::OPERAND_REGISTER, ((0 << 16) | (1 << MCOI::TIED_TO)) }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo66[] = { { RISCV::SPRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::SPRegClassID, 0, MCOI::OPERAND_REGISTER, ((0 << 16) | (1 << MCOI::TIED_TO)) }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo67[] = { { RISCV::GPRCRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::SPRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo68[] = { { RISCV::GPRX0RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRX0RegClassID, 0, MCOI::OPERAND_REGISTER, ((0 << 16) | (1 << MCOI::TIED_TO)) }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo69[] = { { RISCV::GPRCRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRCRegClassID, 0, MCOI::OPERAND_REGISTER, ((0 << 16) | (1 << MCOI::TIED_TO)) }, { RISCV::GPRCRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo70[] = { { RISCV::GPRX0RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRX0RegClassID, 0, MCOI::OPERAND_REGISTER, ((0 << 16) | (1 << MCOI::TIED_TO)) }, { RISCV::GPRNoX0RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo71[] = { { RISCV::GPRCRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRCRegClassID, 0, MCOI::OPERAND_REGISTER, ((0 << 16) | (1 << MCOI::TIED_TO)) }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo72[] = { { RISCV::GPRCRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo73[] = { { RISCV::FPR64CRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRCRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo74[] = { { RISCV::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::SPRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo75[] = { { RISCV::FPR32CRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRCRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo76[] = { { RISCV::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::SPRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo77[] = { { RISCV::GPRNoX0RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo78[] = { { RISCV::GPRCRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRCRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo79[] = { { RISCV::GPRNoX0RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::SPRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo80[] = { { RISCV::GPRNoX0RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo81[] = { { RISCV::GPRX0RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo82[] = { { RISCV::GPRNoX0X2RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo83[] = { { RISCV::GPRNoX0RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRNoX0RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo84[] = { { RISCV::GPRX0RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRNoX0RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo85[] = { { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::SPRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo86[] = { { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, ((0 << 16) | (1 << MCOI::TIED_TO)) }, };
static const MCOperandInfo OperandInfo87[] = { { RISCV::GPRCRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRCRegClassID, 0, MCOI::OPERAND_REGISTER, ((0 << 16) | (1 << MCOI::TIED_TO)) }, };
static const MCOperandInfo OperandInfo88[] = { { RISCV::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo89[] = { { RISCV::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo90[] = { { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo91[] = { { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo92[] = { { RISCV::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo93[] = { { RISCV::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo94[] = { { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo95[] = { { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo96[] = { { RISCV::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo97[] = { { -1, 0, RISCVOp::OPERAND_UIMM4, 0 }, { -1, 0, RISCVOp::OPERAND_UIMM4, 0 }, };
static const MCOperandInfo OperandInfo98[] = { { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo99[] = { { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo100[] = { { RISCV::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, RISCVOp::OPERAND_SIMM12, 0 }, };
static const MCOperandInfo OperandInfo101[] = { { RISCV::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, RISCVOp::OPERAND_SIMM12, 0 }, };
static const MCOperandInfo OperandInfo102[] = { { RISCV::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo103[] = { { RISCV::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo104[] = { { RISCV::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo105[] = { { RISCV::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo106[] = { { RISCV::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo107[] = { { RISCV::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo108[] = { { RISCV::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo109[] = { { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, RISCVOp::OPERAND_SIMM21_LSB0, 0 }, };
static const MCOperandInfo OperandInfo110[] = { { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, RISCVOp::OPERAND_UIMMLOG2XLEN, 0 }, };
static const MCOperandInfo OperandInfo111[] = { { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { RISCV::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, RISCVOp::OPERAND_UIMM5, 0 }, };

extern const MCInstrDesc RISCVInsts[] = {
  { 0,	1,	1,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Variadic), 0x0ULL, nullptr, nullptr, OperandInfo2, -1 ,nullptr },  // Inst #0 = PHI
  { 1,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Variadic), 0x0ULL, nullptr, nullptr, nullptr, -1 ,nullptr },  // Inst #1 = INLINEASM
  { 2,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Branch)|(1ULL<<MCID::IndirectBranch)|(1ULL<<MCID::Terminator)|(1ULL<<MCID::Variadic), 0x0ULL, nullptr, nullptr, nullptr, -1 ,nullptr },  // Inst #2 = INLINEASM_BR
  { 3,	1,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::NotDuplicable), 0x0ULL, nullptr, nullptr, OperandInfo3, -1 ,nullptr },  // Inst #3 = CFI_INSTRUCTION
  { 4,	1,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::NotDuplicable), 0x0ULL, nullptr, nullptr, OperandInfo3, -1 ,nullptr },  // Inst #4 = EH_LABEL
  { 5,	1,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::NotDuplicable), 0x0ULL, nullptr, nullptr, OperandInfo3, -1 ,nullptr },  // Inst #5 = GC_LABEL
  { 6,	1,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::NotDuplicable), 0x0ULL, nullptr, nullptr, OperandInfo3, -1 ,nullptr },  // Inst #6 = ANNOTATION_LABEL
  { 7,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Variadic), 0x0ULL, nullptr, nullptr, nullptr, -1 ,nullptr },  // Inst #7 = KILL
  { 8,	3,	1,	0,	0,	0|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo4, -1 ,nullptr },  // Inst #8 = EXTRACT_SUBREG
  { 9,	4,	1,	0,	0,	0|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo5, -1 ,nullptr },  // Inst #9 = INSERT_SUBREG
  { 10,	1,	1,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Rematerializable)|(1ULL<<MCID::CheapAsAMove), 0x0ULL, nullptr, nullptr, OperandInfo2, -1 ,nullptr },  // Inst #10 = IMPLICIT_DEF
  { 11,	4,	1,	0,	0,	0|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo6, -1 ,nullptr },  // Inst #11 = SUBREG_TO_REG
  { 12,	3,	1,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::CheapAsAMove), 0x0ULL, nullptr, nullptr, OperandInfo4, -1 ,nullptr },  // Inst #12 = COPY_TO_REGCLASS
  { 13,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Variadic), 0x0ULL, nullptr, nullptr, nullptr, -1 ,nullptr },  // Inst #13 = DBG_VALUE
  { 14,	1,	0,	0,	0,	0|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo2, -1 ,nullptr },  // Inst #14 = DBG_LABEL
  { 15,	2,	1,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Variadic)|(1ULL<<MCID::CheapAsAMove), 0x0ULL, nullptr, nullptr, OperandInfo7, -1 ,nullptr },  // Inst #15 = REG_SEQUENCE
  { 16,	2,	1,	0,	65,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::CheapAsAMove), 0x0ULL, nullptr, nullptr, OperandInfo7, -1 ,nullptr },  // Inst #16 = COPY
  { 17,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Variadic), 0x0ULL, nullptr, nullptr, nullptr, -1 ,nullptr },  // Inst #17 = BUNDLE
  { 18,	1,	0,	0,	0,	0|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo3, -1 ,nullptr },  // Inst #18 = LIFETIME_START
  { 19,	1,	0,	0,	0,	0|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo3, -1 ,nullptr },  // Inst #19 = LIFETIME_END
  { 20,	2,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Call)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::UsesCustomInserter)|(1ULL<<MCID::Variadic)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr, OperandInfo8, -1 ,nullptr },  // Inst #20 = STACKMAP
  { 21,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UsesCustomInserter)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr, nullptr, -1 ,nullptr },  // Inst #21 = FENTRY_CALL
  { 22,	6,	1,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Call)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::UsesCustomInserter)|(1ULL<<MCID::Variadic)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr, OperandInfo9, -1 ,nullptr },  // Inst #22 = PATCHPOINT
  { 23,	1,	1,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::Rematerializable), 0x0ULL, nullptr, nullptr, OperandInfo10, -1 ,nullptr },  // Inst #23 = LOAD_STACK_GUARD
  { 24,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Call)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UsesCustomInserter)|(1ULL<<MCID::Variadic)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr, nullptr, -1 ,nullptr },  // Inst #24 = STATEPOINT
  { 25,	2,	0,	0,	0,	0|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo11, -1 ,nullptr },  // Inst #25 = LOCAL_ESCAPE
  { 26,	1,	1,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Branch)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::Terminator)|(1ULL<<MCID::UsesCustomInserter)|(1ULL<<MCID::Variadic)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr, OperandInfo2, -1 ,nullptr },  // Inst #26 = FAULTING_OP
  { 27,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UsesCustomInserter)|(1ULL<<MCID::Variadic)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr, nullptr, -1 ,nullptr },  // Inst #27 = PATCHABLE_OP
  { 28,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::UsesCustomInserter)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr, nullptr, -1 ,nullptr },  // Inst #28 = PATCHABLE_FUNCTION_ENTER
  { 29,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Return)|(1ULL<<MCID::Terminator)|(1ULL<<MCID::UsesCustomInserter)|(1ULL<<MCID::Variadic)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr, nullptr, -1 ,nullptr },  // Inst #29 = PATCHABLE_RET
  { 30,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::UsesCustomInserter)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr, nullptr, -1 ,nullptr },  // Inst #30 = PATCHABLE_FUNCTION_EXIT
  { 31,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Return)|(1ULL<<MCID::UsesCustomInserter)|(1ULL<<MCID::Variadic)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr, nullptr, -1 ,nullptr },  // Inst #31 = PATCHABLE_TAIL_CALL
  { 32,	2,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Call)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UsesCustomInserter)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr, OperandInfo12, -1 ,nullptr },  // Inst #32 = PATCHABLE_EVENT_CALL
  { 33,	3,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Call)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UsesCustomInserter)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr, OperandInfo13, -1 ,nullptr },  // Inst #33 = PATCHABLE_TYPED_EVENT_CALL
  { 34,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Variadic)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr, nullptr, -1 ,nullptr },  // Inst #34 = ICALL_BRANCH_FUNNEL
  { 35,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, nullptr, OperandInfo14, -1 ,nullptr },  // Inst #35 = G_ADD
  { 36,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo14, -1 ,nullptr },  // Inst #36 = G_SUB
  { 37,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, nullptr, OperandInfo14, -1 ,nullptr },  // Inst #37 = G_MUL
  { 38,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo14, -1 ,nullptr },  // Inst #38 = G_SDIV
  { 39,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo14, -1 ,nullptr },  // Inst #39 = G_UDIV
  { 40,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo14, -1 ,nullptr },  // Inst #40 = G_SREM
  { 41,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo14, -1 ,nullptr },  // Inst #41 = G_UREM
  { 42,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, nullptr, OperandInfo14, -1 ,nullptr },  // Inst #42 = G_AND
  { 43,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, nullptr, OperandInfo14, -1 ,nullptr },  // Inst #43 = G_OR
  { 44,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, nullptr, OperandInfo14, -1 ,nullptr },  // Inst #44 = G_XOR
  { 45,	1,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo15, -1 ,nullptr },  // Inst #45 = G_IMPLICIT_DEF
  { 46,	1,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Variadic), 0x0ULL, nullptr, nullptr, OperandInfo15, -1 ,nullptr },  // Inst #46 = G_PHI
  { 47,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo16, -1 ,nullptr },  // Inst #47 = G_FRAME_INDEX
  { 48,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo16, -1 ,nullptr },  // Inst #48 = G_GLOBAL_VALUE
  { 49,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo17, -1 ,nullptr },  // Inst #49 = G_EXTRACT
  { 50,	2,	2,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Variadic), 0x0ULL, nullptr, nullptr, OperandInfo18, -1 ,nullptr },  // Inst #50 = G_UNMERGE_VALUES
  { 51,	4,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo19, -1 ,nullptr },  // Inst #51 = G_INSERT
  { 52,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Variadic), 0x0ULL, nullptr, nullptr, OperandInfo18, -1 ,nullptr },  // Inst #52 = G_MERGE_VALUES
  { 53,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Variadic), 0x0ULL, nullptr, nullptr, OperandInfo18, -1 ,nullptr },  // Inst #53 = G_BUILD_VECTOR
  { 54,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Variadic), 0x0ULL, nullptr, nullptr, OperandInfo18, -1 ,nullptr },  // Inst #54 = G_BUILD_VECTOR_TRUNC
  { 55,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Variadic), 0x0ULL, nullptr, nullptr, OperandInfo18, -1 ,nullptr },  // Inst #55 = G_CONCAT_VECTORS
  { 56,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo18, -1 ,nullptr },  // Inst #56 = G_PTRTOINT
  { 57,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo18, -1 ,nullptr },  // Inst #57 = G_INTTOPTR
  { 58,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo18, -1 ,nullptr },  // Inst #58 = G_BITCAST
  { 59,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo20, -1 ,nullptr },  // Inst #59 = G_INTRINSIC_TRUNC
  { 60,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo20, -1 ,nullptr },  // Inst #60 = G_INTRINSIC_ROUND
  { 61,	1,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr, OperandInfo15, -1 ,nullptr },  // Inst #61 = G_READCYCLECOUNTER
  { 62,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, nullptr, OperandInfo18, -1 ,nullptr },  // Inst #62 = G_LOAD
  { 63,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, nullptr, OperandInfo18, -1 ,nullptr },  // Inst #63 = G_SEXTLOAD
  { 64,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, nullptr, OperandInfo18, -1 ,nullptr },  // Inst #64 = G_ZEXTLOAD
  { 65,	5,	2,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, nullptr, OperandInfo21, -1 ,nullptr },  // Inst #65 = G_INDEXED_LOAD
  { 66,	5,	2,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, nullptr, OperandInfo21, -1 ,nullptr },  // Inst #66 = G_INDEXED_SEXTLOAD
  { 67,	5,	2,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, nullptr, OperandInfo21, -1 ,nullptr },  // Inst #67 = G_INDEXED_ZEXTLOAD
  { 68,	2,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo18, -1 ,nullptr },  // Inst #68 = G_STORE
  { 69,	5,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo22, -1 ,nullptr },  // Inst #69 = G_INDEXED_STORE
  { 70,	5,	2,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo23, -1 ,nullptr },  // Inst #70 = G_ATOMIC_CMPXCHG_WITH_SUCCESS
  { 71,	4,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo24, -1 ,nullptr },  // Inst #71 = G_ATOMIC_CMPXCHG
  { 72,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo25, -1 ,nullptr },  // Inst #72 = G_ATOMICRMW_XCHG
  { 73,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo25, -1 ,nullptr },  // Inst #73 = G_ATOMICRMW_ADD
  { 74,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo25, -1 ,nullptr },  // Inst #74 = G_ATOMICRMW_SUB
  { 75,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo25, -1 ,nullptr },  // Inst #75 = G_ATOMICRMW_AND
  { 76,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo25, -1 ,nullptr },  // Inst #76 = G_ATOMICRMW_NAND
  { 77,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo25, -1 ,nullptr },  // Inst #77 = G_ATOMICRMW_OR
  { 78,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo25, -1 ,nullptr },  // Inst #78 = G_ATOMICRMW_XOR
  { 79,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo25, -1 ,nullptr },  // Inst #79 = G_ATOMICRMW_MAX
  { 80,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo25, -1 ,nullptr },  // Inst #80 = G_ATOMICRMW_MIN
  { 81,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo25, -1 ,nullptr },  // Inst #81 = G_ATOMICRMW_UMAX
  { 82,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo25, -1 ,nullptr },  // Inst #82 = G_ATOMICRMW_UMIN
  { 83,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo25, -1 ,nullptr },  // Inst #83 = G_ATOMICRMW_FADD
  { 84,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo25, -1 ,nullptr },  // Inst #84 = G_ATOMICRMW_FSUB
  { 85,	2,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr, OperandInfo8, -1 ,nullptr },  // Inst #85 = G_FENCE
  { 86,	2,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Branch)|(1ULL<<MCID::Terminator), 0x0ULL, nullptr, nullptr, OperandInfo16, -1 ,nullptr },  // Inst #86 = G_BRCOND
  { 87,	1,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Branch)|(1ULL<<MCID::Terminator), 0x0ULL, nullptr, nullptr, OperandInfo15, -1 ,nullptr },  // Inst #87 = G_BRINDIRECT
  { 88,	1,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Variadic), 0x0ULL, nullptr, nullptr, OperandInfo2, -1 ,nullptr },  // Inst #88 = G_INTRINSIC
  { 89,	1,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::Variadic)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr, OperandInfo2, -1 ,nullptr },  // Inst #89 = G_INTRINSIC_W_SIDE_EFFECTS
  { 90,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo18, -1 ,nullptr },  // Inst #90 = G_ANYEXT
  { 91,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo18, -1 ,nullptr },  // Inst #91 = G_TRUNC
  { 92,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo16, -1 ,nullptr },  // Inst #92 = G_CONSTANT
  { 93,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo16, -1 ,nullptr },  // Inst #93 = G_FCONSTANT
  { 94,	1,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo15, -1 ,nullptr },  // Inst #94 = G_VASTART
  { 95,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo17, -1 ,nullptr },  // Inst #95 = G_VAARG
  { 96,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo18, -1 ,nullptr },  // Inst #96 = G_SEXT
  { 97,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo26, -1 ,nullptr },  // Inst #97 = G_SEXT_INREG
  { 98,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo18, -1 ,nullptr },  // Inst #98 = G_ZEXT
  { 99,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo27, -1 ,nullptr },  // Inst #99 = G_SHL
  { 100,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo27, -1 ,nullptr },  // Inst #100 = G_LSHR
  { 101,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo27, -1 ,nullptr },  // Inst #101 = G_ASHR
  { 102,	4,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo28, -1 ,nullptr },  // Inst #102 = G_ICMP
  { 103,	4,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo28, -1 ,nullptr },  // Inst #103 = G_FCMP
  { 104,	4,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo24, -1 ,nullptr },  // Inst #104 = G_SELECT
  { 105,	4,	2,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, nullptr, OperandInfo24, -1 ,nullptr },  // Inst #105 = G_UADDO
  { 106,	5,	2,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo29, -1 ,nullptr },  // Inst #106 = G_UADDE
  { 107,	4,	2,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo24, -1 ,nullptr },  // Inst #107 = G_USUBO
  { 108,	5,	2,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo29, -1 ,nullptr },  // Inst #108 = G_USUBE
  { 109,	4,	2,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, nullptr, OperandInfo24, -1 ,nullptr },  // Inst #109 = G_SADDO
  { 110,	5,	2,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo29, -1 ,nullptr },  // Inst #110 = G_SADDE
  { 111,	4,	2,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo24, -1 ,nullptr },  // Inst #111 = G_SSUBO
  { 112,	5,	2,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo29, -1 ,nullptr },  // Inst #112 = G_SSUBE
  { 113,	4,	2,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, nullptr, OperandInfo24, -1 ,nullptr },  // Inst #113 = G_UMULO
  { 114,	4,	2,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, nullptr, OperandInfo24, -1 ,nullptr },  // Inst #114 = G_SMULO
  { 115,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, nullptr, OperandInfo14, -1 ,nullptr },  // Inst #115 = G_UMULH
  { 116,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, nullptr, OperandInfo14, -1 ,nullptr },  // Inst #116 = G_SMULH
  { 117,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, nullptr, OperandInfo14, -1 ,nullptr },  // Inst #117 = G_FADD
  { 118,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo14, -1 ,nullptr },  // Inst #118 = G_FSUB
  { 119,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, nullptr, OperandInfo14, -1 ,nullptr },  // Inst #119 = G_FMUL
  { 120,	4,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo30, -1 ,nullptr },  // Inst #120 = G_FMA
  { 121,	4,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo30, -1 ,nullptr },  // Inst #121 = G_FMAD
  { 122,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo14, -1 ,nullptr },  // Inst #122 = G_FDIV
  { 123,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo14, -1 ,nullptr },  // Inst #123 = G_FREM
  { 124,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo14, -1 ,nullptr },  // Inst #124 = G_FPOW
  { 125,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo20, -1 ,nullptr },  // Inst #125 = G_FEXP
  { 126,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo20, -1 ,nullptr },  // Inst #126 = G_FEXP2
  { 127,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo20, -1 ,nullptr },  // Inst #127 = G_FLOG
  { 128,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo20, -1 ,nullptr },  // Inst #128 = G_FLOG2
  { 129,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo20, -1 ,nullptr },  // Inst #129 = G_FLOG10
  { 130,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo20, -1 ,nullptr },  // Inst #130 = G_FNEG
  { 131,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo18, -1 ,nullptr },  // Inst #131 = G_FPEXT
  { 132,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo18, -1 ,nullptr },  // Inst #132 = G_FPTRUNC
  { 133,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo18, -1 ,nullptr },  // Inst #133 = G_FPTOSI
  { 134,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo18, -1 ,nullptr },  // Inst #134 = G_FPTOUI
  { 135,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo18, -1 ,nullptr },  // Inst #135 = G_SITOFP
  { 136,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo18, -1 ,nullptr },  // Inst #136 = G_UITOFP
  { 137,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo20, -1 ,nullptr },  // Inst #137 = G_FABS
  { 138,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo27, -1 ,nullptr },  // Inst #138 = G_FCOPYSIGN
  { 139,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo20, -1 ,nullptr },  // Inst #139 = G_FCANONICALIZE
  { 140,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, nullptr, OperandInfo14, -1 ,nullptr },  // Inst #140 = G_FMINNUM
  { 141,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, nullptr, OperandInfo14, -1 ,nullptr },  // Inst #141 = G_FMAXNUM
  { 142,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, nullptr, OperandInfo14, -1 ,nullptr },  // Inst #142 = G_FMINNUM_IEEE
  { 143,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, nullptr, OperandInfo14, -1 ,nullptr },  // Inst #143 = G_FMAXNUM_IEEE
  { 144,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, nullptr, OperandInfo14, -1 ,nullptr },  // Inst #144 = G_FMINIMUM
  { 145,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, nullptr, OperandInfo14, -1 ,nullptr },  // Inst #145 = G_FMAXIMUM
  { 146,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo27, -1 ,nullptr },  // Inst #146 = G_PTR_ADD
  { 147,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo31, -1 ,nullptr },  // Inst #147 = G_PTR_MASK
  { 148,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, nullptr, OperandInfo14, -1 ,nullptr },  // Inst #148 = G_SMIN
  { 149,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, nullptr, OperandInfo14, -1 ,nullptr },  // Inst #149 = G_SMAX
  { 150,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, nullptr, OperandInfo14, -1 ,nullptr },  // Inst #150 = G_UMIN
  { 151,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, nullptr, OperandInfo14, -1 ,nullptr },  // Inst #151 = G_UMAX
  { 152,	1,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Branch)|(1ULL<<MCID::Barrier)|(1ULL<<MCID::Terminator), 0x0ULL, nullptr, nullptr, OperandInfo2, -1 ,nullptr },  // Inst #152 = G_BR
  { 153,	3,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Branch)|(1ULL<<MCID::Terminator), 0x0ULL, nullptr, nullptr, OperandInfo32, -1 ,nullptr },  // Inst #153 = G_BRJT
  { 154,	4,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo33, -1 ,nullptr },  // Inst #154 = G_INSERT_VECTOR_ELT
  { 155,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo34, -1 ,nullptr },  // Inst #155 = G_EXTRACT_VECTOR_ELT
  { 156,	4,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo35, -1 ,nullptr },  // Inst #156 = G_SHUFFLE_VECTOR
  { 157,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo18, -1 ,nullptr },  // Inst #157 = G_CTTZ
  { 158,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo18, -1 ,nullptr },  // Inst #158 = G_CTTZ_ZERO_UNDEF
  { 159,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo18, -1 ,nullptr },  // Inst #159 = G_CTLZ
  { 160,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo18, -1 ,nullptr },  // Inst #160 = G_CTLZ_ZERO_UNDEF
  { 161,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo18, -1 ,nullptr },  // Inst #161 = G_CTPOP
  { 162,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo20, -1 ,nullptr },  // Inst #162 = G_BSWAP
  { 163,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo20, -1 ,nullptr },  // Inst #163 = G_BITREVERSE
  { 164,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo20, -1 ,nullptr },  // Inst #164 = G_FCEIL
  { 165,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo20, -1 ,nullptr },  // Inst #165 = G_FCOS
  { 166,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo20, -1 ,nullptr },  // Inst #166 = G_FSIN
  { 167,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo20, -1 ,nullptr },  // Inst #167 = G_FSQRT
  { 168,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo20, -1 ,nullptr },  // Inst #168 = G_FFLOOR
  { 169,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo20, -1 ,nullptr },  // Inst #169 = G_FRINT
  { 170,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo20, -1 ,nullptr },  // Inst #170 = G_FNEARBYINT
  { 171,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo18, -1 ,nullptr },  // Inst #171 = G_ADDRSPACE_CAST
  { 172,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo16, -1 ,nullptr },  // Inst #172 = G_BLOCK_ADDR
  { 173,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo16, -1 ,nullptr },  // Inst #173 = G_JUMP_TABLE
  { 174,	3,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr, OperandInfo36, -1 ,nullptr },  // Inst #174 = G_DYN_STACKALLOC
  { 175,	2,	1,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::UnmodeledSideEffects)|(1ULL<<MCID::Convergent), 0x0ULL, nullptr, nullptr, OperandInfo16, -1 ,nullptr },  // Inst #175 = G_READ_REGISTER
  { 176,	2,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::UnmodeledSideEffects)|(1ULL<<MCID::Convergent), 0x0ULL, nullptr, nullptr, OperandInfo37, -1 ,nullptr },  // Inst #176 = G_WRITE_REGISTER
  { 177,	2,	0,	4,	0,	0|(1ULL<<MCID::Pseudo), 0x0ULL, ImplicitList1, ImplicitList1, OperandInfo8, -1 ,nullptr },  // Inst #177 = ADJCALLSTACKDOWN
  { 178,	2,	0,	4,	0,	0|(1ULL<<MCID::Pseudo), 0x0ULL, ImplicitList1, ImplicitList1, OperandInfo8, -1 ,nullptr },  // Inst #178 = ADJCALLSTACKUP
  { 179,	3,	1,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::UsesCustomInserter), 0x0ULL, nullptr, nullptr, OperandInfo38, -1 ,nullptr },  // Inst #179 = BuildPairF64Pseudo
  { 180,	4,	1,	4,	0,	0|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo39, -1 ,nullptr },  // Inst #180 = PseudoAddTPRel
  { 181,	5,	2,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo40, -1 ,nullptr },  // Inst #181 = PseudoAtomicLoadNand32
  { 182,	5,	2,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo40, -1 ,nullptr },  // Inst #182 = PseudoAtomicLoadNand64
  { 183,	1,	0,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Branch)|(1ULL<<MCID::Barrier)|(1ULL<<MCID::Terminator), 0x0ULL, nullptr, nullptr, OperandInfo41, -1 ,nullptr },  // Inst #183 = PseudoBR
  { 184,	2,	0,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Branch)|(1ULL<<MCID::IndirectBranch)|(1ULL<<MCID::Barrier)|(1ULL<<MCID::Call)|(1ULL<<MCID::Terminator), 0x0ULL, nullptr, ImplicitList2, OperandInfo42, -1 ,nullptr },  // Inst #184 = PseudoBRIND
  { 185,	1,	0,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Call), 0x0ULL, nullptr, ImplicitList2, OperandInfo2, -1 ,nullptr },  // Inst #185 = PseudoCALL
  { 186,	1,	0,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Call), 0x0ULL, nullptr, ImplicitList2, OperandInfo43, -1 ,nullptr },  // Inst #186 = PseudoCALLIndirect
  { 187,	2,	1,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Barrier)|(1ULL<<MCID::Call), 0x0ULL, nullptr, nullptr, OperandInfo44, -1 ,nullptr },  // Inst #187 = PseudoCALLReg
  { 188,	6,	2,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo45, -1 ,nullptr },  // Inst #188 = PseudoCmpXchg32
  { 189,	6,	2,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo45, -1 ,nullptr },  // Inst #189 = PseudoCmpXchg64
  { 190,	3,	2,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, nullptr, OperandInfo46, -1 ,nullptr },  // Inst #190 = PseudoFLD
  { 191,	3,	2,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, nullptr, OperandInfo47, -1 ,nullptr },  // Inst #191 = PseudoFLW
  { 192,	3,	2,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo46, -1 ,nullptr },  // Inst #192 = PseudoFSD
  { 193,	3,	2,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo47, -1 ,nullptr },  // Inst #193 = PseudoFSW
  { 194,	2,	1,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, nullptr, OperandInfo44, -1 ,nullptr },  // Inst #194 = PseudoLA
  { 195,	2,	1,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, nullptr, OperandInfo44, -1 ,nullptr },  // Inst #195 = PseudoLA_TLS_GD
  { 196,	2,	1,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, nullptr, OperandInfo44, -1 ,nullptr },  // Inst #196 = PseudoLA_TLS_IE
  { 197,	2,	1,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, nullptr, OperandInfo44, -1 ,nullptr },  // Inst #197 = PseudoLB
  { 198,	2,	1,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, nullptr, OperandInfo44, -1 ,nullptr },  // Inst #198 = PseudoLBU
  { 199,	2,	1,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, nullptr, OperandInfo44, -1 ,nullptr },  // Inst #199 = PseudoLD
  { 200,	2,	1,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, nullptr, OperandInfo44, -1 ,nullptr },  // Inst #200 = PseudoLH
  { 201,	2,	1,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, nullptr, OperandInfo44, -1 ,nullptr },  // Inst #201 = PseudoLHU
  { 202,	2,	1,	32,	0,	0|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo44, -1 ,nullptr },  // Inst #202 = PseudoLI
  { 203,	2,	1,	4,	0,	0|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, nullptr, OperandInfo44, -1 ,nullptr },  // Inst #203 = PseudoLLA
  { 204,	2,	1,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, nullptr, OperandInfo44, -1 ,nullptr },  // Inst #204 = PseudoLW
  { 205,	2,	1,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, nullptr, OperandInfo44, -1 ,nullptr },  // Inst #205 = PseudoLWU
  { 206,	6,	2,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo45, -1 ,nullptr },  // Inst #206 = PseudoMaskedAtomicLoadAdd32
  { 207,	8,	3,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo48, -1 ,nullptr },  // Inst #207 = PseudoMaskedAtomicLoadMax32
  { 208,	8,	3,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo48, -1 ,nullptr },  // Inst #208 = PseudoMaskedAtomicLoadMin32
  { 209,	6,	2,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo45, -1 ,nullptr },  // Inst #209 = PseudoMaskedAtomicLoadNand32
  { 210,	6,	2,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo45, -1 ,nullptr },  // Inst #210 = PseudoMaskedAtomicLoadSub32
  { 211,	7,	3,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo49, -1 ,nullptr },  // Inst #211 = PseudoMaskedAtomicLoadUMax32
  { 212,	7,	3,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo49, -1 ,nullptr },  // Inst #212 = PseudoMaskedAtomicLoadUMin32
  { 213,	6,	2,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo45, -1 ,nullptr },  // Inst #213 = PseudoMaskedAtomicSwap32
  { 214,	7,	2,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo50, -1 ,nullptr },  // Inst #214 = PseudoMaskedCmpXchg32
  { 215,	0,	0,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Return)|(1ULL<<MCID::Barrier)|(1ULL<<MCID::Terminator), 0x0ULL, nullptr, nullptr, nullptr, -1 ,nullptr },  // Inst #215 = PseudoRET
  { 216,	3,	2,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo51, -1 ,nullptr },  // Inst #216 = PseudoSB
  { 217,	3,	2,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo51, -1 ,nullptr },  // Inst #217 = PseudoSD
  { 218,	3,	2,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo51, -1 ,nullptr },  // Inst #218 = PseudoSH
  { 219,	3,	2,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, nullptr, OperandInfo51, -1 ,nullptr },  // Inst #219 = PseudoSW
  { 220,	1,	0,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Return)|(1ULL<<MCID::Barrier)|(1ULL<<MCID::Call)|(1ULL<<MCID::Terminator), 0x0ULL, ImplicitList1, nullptr, OperandInfo2, -1 ,nullptr },  // Inst #220 = PseudoTAIL
  { 221,	1,	0,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Return)|(1ULL<<MCID::Barrier)|(1ULL<<MCID::Call)|(1ULL<<MCID::Terminator), 0x0ULL, ImplicitList1, nullptr, OperandInfo52, -1 ,nullptr },  // Inst #221 = PseudoTAILIndirect
  { 222,	2,	2,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::UsesCustomInserter), 0x0ULL, nullptr, nullptr, OperandInfo53, -1 ,nullptr },  // Inst #222 = ReadCycleWide
  { 223,	6,	1,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::UsesCustomInserter), 0x0ULL, nullptr, nullptr, OperandInfo54, -1 ,nullptr },  // Inst #223 = Select_FPR32_Using_CC_GPR
  { 224,	6,	1,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::UsesCustomInserter), 0x0ULL, nullptr, nullptr, OperandInfo55, -1 ,nullptr },  // Inst #224 = Select_FPR64_Using_CC_GPR
  { 225,	6,	1,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::UsesCustomInserter), 0x0ULL, nullptr, nullptr, OperandInfo56, -1 ,nullptr },  // Inst #225 = Select_GPR_Using_CC_GPR
  { 226,	3,	2,	4,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::UsesCustomInserter), 0x0ULL, nullptr, nullptr, OperandInfo57, -1 ,nullptr },  // Inst #226 = SplitF64Pseudo
  { 227,	3,	1,	4,	1,	0, 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #227 = ADD
  { 228,	3,	1,	4,	2,	0|(1ULL<<MCID::Rematerializable)|(1ULL<<MCID::CheapAsAMove), 0x3ULL, nullptr, nullptr, OperandInfo59, -1 ,nullptr },  // Inst #228 = ADDI
  { 229,	3,	1,	4,	3,	0, 0x3ULL, nullptr, nullptr, OperandInfo59, -1 ,nullptr },  // Inst #229 = ADDIW
  { 230,	3,	1,	4,	4,	0, 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #230 = ADDW
  { 231,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #231 = AMOADD_D
  { 232,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #232 = AMOADD_D_AQ
  { 233,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #233 = AMOADD_D_AQ_RL
  { 234,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #234 = AMOADD_D_RL
  { 235,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #235 = AMOADD_W
  { 236,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #236 = AMOADD_W_AQ
  { 237,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #237 = AMOADD_W_AQ_RL
  { 238,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #238 = AMOADD_W_RL
  { 239,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #239 = AMOAND_D
  { 240,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #240 = AMOAND_D_AQ
  { 241,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #241 = AMOAND_D_AQ_RL
  { 242,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #242 = AMOAND_D_RL
  { 243,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #243 = AMOAND_W
  { 244,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #244 = AMOAND_W_AQ
  { 245,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #245 = AMOAND_W_AQ_RL
  { 246,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #246 = AMOAND_W_RL
  { 247,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #247 = AMOMAXU_D
  { 248,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #248 = AMOMAXU_D_AQ
  { 249,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #249 = AMOMAXU_D_AQ_RL
  { 250,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #250 = AMOMAXU_D_RL
  { 251,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #251 = AMOMAXU_W
  { 252,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #252 = AMOMAXU_W_AQ
  { 253,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #253 = AMOMAXU_W_AQ_RL
  { 254,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #254 = AMOMAXU_W_RL
  { 255,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #255 = AMOMAX_D
  { 256,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #256 = AMOMAX_D_AQ
  { 257,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #257 = AMOMAX_D_AQ_RL
  { 258,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #258 = AMOMAX_D_RL
  { 259,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #259 = AMOMAX_W
  { 260,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #260 = AMOMAX_W_AQ
  { 261,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #261 = AMOMAX_W_AQ_RL
  { 262,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #262 = AMOMAX_W_RL
  { 263,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #263 = AMOMINU_D
  { 264,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #264 = AMOMINU_D_AQ
  { 265,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #265 = AMOMINU_D_AQ_RL
  { 266,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #266 = AMOMINU_D_RL
  { 267,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #267 = AMOMINU_W
  { 268,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #268 = AMOMINU_W_AQ
  { 269,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #269 = AMOMINU_W_AQ_RL
  { 270,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #270 = AMOMINU_W_RL
  { 271,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #271 = AMOMIN_D
  { 272,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #272 = AMOMIN_D_AQ
  { 273,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #273 = AMOMIN_D_AQ_RL
  { 274,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #274 = AMOMIN_D_RL
  { 275,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #275 = AMOMIN_W
  { 276,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #276 = AMOMIN_W_AQ
  { 277,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #277 = AMOMIN_W_AQ_RL
  { 278,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #278 = AMOMIN_W_RL
  { 279,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #279 = AMOOR_D
  { 280,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #280 = AMOOR_D_AQ
  { 281,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #281 = AMOOR_D_AQ_RL
  { 282,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #282 = AMOOR_D_RL
  { 283,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #283 = AMOOR_W
  { 284,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #284 = AMOOR_W_AQ
  { 285,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #285 = AMOOR_W_AQ_RL
  { 286,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #286 = AMOOR_W_RL
  { 287,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #287 = AMOSWAP_D
  { 288,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #288 = AMOSWAP_D_AQ
  { 289,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #289 = AMOSWAP_D_AQ_RL
  { 290,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #290 = AMOSWAP_D_RL
  { 291,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #291 = AMOSWAP_W
  { 292,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #292 = AMOSWAP_W_AQ
  { 293,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #293 = AMOSWAP_W_AQ_RL
  { 294,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #294 = AMOSWAP_W_RL
  { 295,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #295 = AMOXOR_D
  { 296,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #296 = AMOXOR_D_AQ
  { 297,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #297 = AMOXOR_D_AQ_RL
  { 298,	3,	1,	4,	5,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #298 = AMOXOR_D_RL
  { 299,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #299 = AMOXOR_W
  { 300,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #300 = AMOXOR_W_AQ
  { 301,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #301 = AMOXOR_W_AQ_RL
  { 302,	3,	1,	4,	6,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #302 = AMOXOR_W_RL
  { 303,	3,	1,	4,	1,	0, 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #303 = AND
  { 304,	3,	1,	4,	2,	0, 0x3ULL, nullptr, nullptr, OperandInfo59, -1 ,nullptr },  // Inst #304 = ANDI
  { 305,	2,	1,	4,	7,	0, 0x6ULL, nullptr, nullptr, OperandInfo60, -1 ,nullptr },  // Inst #305 = AUIPC
  { 306,	3,	0,	4,	8,	0|(1ULL<<MCID::Branch)|(1ULL<<MCID::Terminator), 0x5ULL, nullptr, nullptr, OperandInfo61, -1 ,nullptr },  // Inst #306 = BEQ
  { 307,	3,	0,	4,	8,	0|(1ULL<<MCID::Branch)|(1ULL<<MCID::Terminator), 0x5ULL, nullptr, nullptr, OperandInfo61, -1 ,nullptr },  // Inst #307 = BGE
  { 308,	3,	0,	4,	8,	0|(1ULL<<MCID::Branch)|(1ULL<<MCID::Terminator), 0x5ULL, nullptr, nullptr, OperandInfo61, -1 ,nullptr },  // Inst #308 = BGEU
  { 309,	3,	0,	4,	8,	0|(1ULL<<MCID::Branch)|(1ULL<<MCID::Terminator), 0x5ULL, nullptr, nullptr, OperandInfo61, -1 ,nullptr },  // Inst #309 = BLT
  { 310,	3,	0,	4,	8,	0|(1ULL<<MCID::Branch)|(1ULL<<MCID::Terminator), 0x5ULL, nullptr, nullptr, OperandInfo61, -1 ,nullptr },  // Inst #310 = BLTU
  { 311,	3,	0,	4,	8,	0|(1ULL<<MCID::Branch)|(1ULL<<MCID::Terminator), 0x5ULL, nullptr, nullptr, OperandInfo61, -1 ,nullptr },  // Inst #311 = BNE
  { 312,	3,	1,	4,	9,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x3ULL, nullptr, nullptr, OperandInfo62, -1 ,nullptr },  // Inst #312 = CSRRC
  { 313,	3,	1,	4,	10,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x3ULL, nullptr, nullptr, OperandInfo63, -1 ,nullptr },  // Inst #313 = CSRRCI
  { 314,	3,	1,	4,	9,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x3ULL, nullptr, nullptr, OperandInfo62, -1 ,nullptr },  // Inst #314 = CSRRS
  { 315,	3,	1,	4,	10,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x3ULL, nullptr, nullptr, OperandInfo63, -1 ,nullptr },  // Inst #315 = CSRRSI
  { 316,	3,	1,	4,	9,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x3ULL, nullptr, nullptr, OperandInfo62, -1 ,nullptr },  // Inst #316 = CSRRW
  { 317,	3,	1,	4,	10,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x3ULL, nullptr, nullptr, OperandInfo63, -1 ,nullptr },  // Inst #317 = CSRRWI
  { 318,	3,	1,	2,	1,	0, 0x8ULL, nullptr, nullptr, OperandInfo64, -1 ,nullptr },  // Inst #318 = C_ADD
  { 319,	3,	1,	2,	2,	0, 0x9ULL, nullptr, nullptr, OperandInfo65, -1 ,nullptr },  // Inst #319 = C_ADDI
  { 320,	3,	1,	2,	2,	0, 0x9ULL, nullptr, nullptr, OperandInfo66, -1 ,nullptr },  // Inst #320 = C_ADDI16SP
  { 321,	3,	1,	2,	2,	0, 0xbULL, ImplicitList1, nullptr, OperandInfo67, -1 ,nullptr },  // Inst #321 = C_ADDI4SPN
  { 322,	3,	1,	2,	3,	0, 0x9ULL, nullptr, nullptr, OperandInfo65, -1 ,nullptr },  // Inst #322 = C_ADDIW
  { 323,	3,	1,	2,	2,	0, 0x9ULL, nullptr, nullptr, OperandInfo65, -1 ,nullptr },  // Inst #323 = C_ADDI_HINT_IMM_ZERO
  { 324,	3,	1,	2,	2,	0, 0x9ULL, nullptr, nullptr, OperandInfo68, -1 ,nullptr },  // Inst #324 = C_ADDI_HINT_X0
  { 325,	3,	1,	2,	2,	0, 0x9ULL, nullptr, nullptr, OperandInfo68, -1 ,nullptr },  // Inst #325 = C_ADDI_NOP
  { 326,	3,	1,	2,	4,	0, 0xeULL, nullptr, nullptr, OperandInfo69, -1 ,nullptr },  // Inst #326 = C_ADDW
  { 327,	3,	1,	2,	1,	0, 0x8ULL, nullptr, nullptr, OperandInfo70, -1 ,nullptr },  // Inst #327 = C_ADD_HINT
  { 328,	3,	1,	2,	1,	0, 0xeULL, nullptr, nullptr, OperandInfo69, -1 ,nullptr },  // Inst #328 = C_AND
  { 329,	3,	1,	2,	2,	0, 0xfULL, nullptr, nullptr, OperandInfo71, -1 ,nullptr },  // Inst #329 = C_ANDI
  { 330,	2,	0,	2,	8,	0|(1ULL<<MCID::Branch)|(1ULL<<MCID::Terminator), 0xfULL, nullptr, nullptr, OperandInfo72, -1 ,nullptr },  // Inst #330 = C_BEQZ
  { 331,	2,	0,	2,	8,	0|(1ULL<<MCID::Branch)|(1ULL<<MCID::Terminator), 0xfULL, nullptr, nullptr, OperandInfo72, -1 ,nullptr },  // Inst #331 = C_BNEZ
  { 332,	0,	0,	2,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x8ULL, nullptr, nullptr, nullptr, -1 ,nullptr },  // Inst #332 = C_EBREAK
  { 333,	3,	1,	2,	11,	0|(1ULL<<MCID::MayLoad), 0xcULL, nullptr, nullptr, OperandInfo73, -1 ,nullptr },  // Inst #333 = C_FLD
  { 334,	3,	1,	2,	11,	0|(1ULL<<MCID::MayLoad), 0x9ULL, nullptr, nullptr, OperandInfo74, -1 ,nullptr },  // Inst #334 = C_FLDSP
  { 335,	3,	1,	2,	12,	0|(1ULL<<MCID::MayLoad), 0xcULL, nullptr, nullptr, OperandInfo75, -1 ,nullptr },  // Inst #335 = C_FLW
  { 336,	3,	1,	2,	12,	0|(1ULL<<MCID::MayLoad), 0x9ULL, nullptr, nullptr, OperandInfo76, -1 ,nullptr },  // Inst #336 = C_FLWSP
  { 337,	3,	0,	2,	13,	0|(1ULL<<MCID::MayStore), 0xdULL, nullptr, nullptr, OperandInfo73, -1 ,nullptr },  // Inst #337 = C_FSD
  { 338,	3,	0,	2,	13,	0|(1ULL<<MCID::MayStore), 0xaULL, nullptr, nullptr, OperandInfo74, -1 ,nullptr },  // Inst #338 = C_FSDSP
  { 339,	3,	0,	2,	14,	0|(1ULL<<MCID::MayStore), 0xdULL, nullptr, nullptr, OperandInfo75, -1 ,nullptr },  // Inst #339 = C_FSW
  { 340,	3,	0,	2,	14,	0|(1ULL<<MCID::MayStore), 0xaULL, nullptr, nullptr, OperandInfo76, -1 ,nullptr },  // Inst #340 = C_FSWSP
  { 341,	1,	0,	2,	8,	0|(1ULL<<MCID::Branch)|(1ULL<<MCID::Barrier)|(1ULL<<MCID::Terminator), 0x10ULL, nullptr, nullptr, OperandInfo2, -1 ,nullptr },  // Inst #341 = C_J
  { 342,	1,	0,	2,	15,	0|(1ULL<<MCID::Call), 0x10ULL, nullptr, ImplicitList2, OperandInfo2, -1 ,nullptr },  // Inst #342 = C_JAL
  { 343,	1,	0,	2,	16,	0|(1ULL<<MCID::Call), 0x8ULL, nullptr, ImplicitList2, OperandInfo77, -1 ,nullptr },  // Inst #343 = C_JALR
  { 344,	1,	0,	2,	17,	0|(1ULL<<MCID::Branch)|(1ULL<<MCID::IndirectBranch)|(1ULL<<MCID::Barrier)|(1ULL<<MCID::Terminator), 0x8ULL, nullptr, nullptr, OperandInfo77, -1 ,nullptr },  // Inst #344 = C_JR
  { 345,	3,	1,	2,	18,	0|(1ULL<<MCID::MayLoad), 0xcULL, nullptr, nullptr, OperandInfo78, -1 ,nullptr },  // Inst #345 = C_LD
  { 346,	3,	1,	2,	18,	0|(1ULL<<MCID::MayLoad), 0x9ULL, nullptr, nullptr, OperandInfo79, -1 ,nullptr },  // Inst #346 = C_LDSP
  { 347,	2,	1,	2,	7,	0, 0x9ULL, nullptr, nullptr, OperandInfo80, -1 ,nullptr },  // Inst #347 = C_LI
  { 348,	2,	1,	2,	7,	0, 0x9ULL, nullptr, nullptr, OperandInfo81, -1 ,nullptr },  // Inst #348 = C_LI_HINT
  { 349,	2,	1,	2,	7,	0, 0x9ULL, nullptr, nullptr, OperandInfo82, -1 ,nullptr },  // Inst #349 = C_LUI
  { 350,	2,	1,	2,	7,	0, 0x9ULL, nullptr, nullptr, OperandInfo81, -1 ,nullptr },  // Inst #350 = C_LUI_HINT
  { 351,	3,	1,	2,	19,	0|(1ULL<<MCID::MayLoad), 0xcULL, nullptr, nullptr, OperandInfo78, -1 ,nullptr },  // Inst #351 = C_LW
  { 352,	3,	1,	2,	19,	0|(1ULL<<MCID::MayLoad), 0x9ULL, nullptr, nullptr, OperandInfo79, -1 ,nullptr },  // Inst #352 = C_LWSP
  { 353,	2,	1,	2,	2,	0, 0x8ULL, nullptr, nullptr, OperandInfo83, -1 ,nullptr },  // Inst #353 = C_MV
  { 354,	2,	1,	2,	2,	0, 0x8ULL, nullptr, nullptr, OperandInfo84, -1 ,nullptr },  // Inst #354 = C_MV_HINT
  { 355,	0,	0,	2,	20,	0, 0x9ULL, nullptr, nullptr, nullptr, -1 ,nullptr },  // Inst #355 = C_NOP
  { 356,	1,	0,	2,	20,	0, 0x9ULL, nullptr, nullptr, OperandInfo2, -1 ,nullptr },  // Inst #356 = C_NOP_HINT
  { 357,	3,	1,	2,	1,	0, 0xeULL, nullptr, nullptr, OperandInfo69, -1 ,nullptr },  // Inst #357 = C_OR
  { 358,	3,	0,	2,	21,	0|(1ULL<<MCID::MayStore), 0xdULL, nullptr, nullptr, OperandInfo78, -1 ,nullptr },  // Inst #358 = C_SD
  { 359,	3,	0,	2,	21,	0|(1ULL<<MCID::MayStore), 0xaULL, nullptr, nullptr, OperandInfo85, -1 ,nullptr },  // Inst #359 = C_SDSP
  { 360,	3,	1,	2,	22,	0, 0x9ULL, nullptr, nullptr, OperandInfo65, -1 ,nullptr },  // Inst #360 = C_SLLI
  { 361,	2,	1,	2,	22,	0, 0x9ULL, nullptr, nullptr, OperandInfo86, -1 ,nullptr },  // Inst #361 = C_SLLI64_HINT
  { 362,	3,	1,	2,	22,	0, 0x9ULL, nullptr, nullptr, OperandInfo68, -1 ,nullptr },  // Inst #362 = C_SLLI_HINT
  { 363,	3,	1,	2,	22,	0, 0xfULL, nullptr, nullptr, OperandInfo71, -1 ,nullptr },  // Inst #363 = C_SRAI
  { 364,	2,	1,	2,	22,	0, 0x9ULL, nullptr, nullptr, OperandInfo87, -1 ,nullptr },  // Inst #364 = C_SRAI64_HINT
  { 365,	3,	1,	2,	22,	0, 0xfULL, nullptr, nullptr, OperandInfo71, -1 ,nullptr },  // Inst #365 = C_SRLI
  { 366,	2,	1,	2,	22,	0, 0x9ULL, nullptr, nullptr, OperandInfo87, -1 ,nullptr },  // Inst #366 = C_SRLI64_HINT
  { 367,	3,	1,	2,	1,	0, 0xeULL, nullptr, nullptr, OperandInfo69, -1 ,nullptr },  // Inst #367 = C_SUB
  { 368,	3,	1,	2,	4,	0, 0xeULL, nullptr, nullptr, OperandInfo69, -1 ,nullptr },  // Inst #368 = C_SUBW
  { 369,	3,	0,	2,	23,	0|(1ULL<<MCID::MayStore), 0xdULL, nullptr, nullptr, OperandInfo78, -1 ,nullptr },  // Inst #369 = C_SW
  { 370,	3,	0,	2,	23,	0|(1ULL<<MCID::MayStore), 0xaULL, nullptr, nullptr, OperandInfo85, -1 ,nullptr },  // Inst #370 = C_SWSP
  { 371,	0,	0,	2,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x11ULL, nullptr, nullptr, nullptr, -1 ,nullptr },  // Inst #371 = C_UNIMP
  { 372,	3,	1,	2,	1,	0, 0xeULL, nullptr, nullptr, OperandInfo69, -1 ,nullptr },  // Inst #372 = C_XOR
  { 373,	3,	1,	4,	24,	0, 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #373 = DIV
  { 374,	3,	1,	4,	24,	0, 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #374 = DIVU
  { 375,	3,	1,	4,	25,	0, 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #375 = DIVUW
  { 376,	3,	1,	4,	25,	0, 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #376 = DIVW
  { 377,	0,	0,	4,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x3ULL, nullptr, nullptr, nullptr, -1 ,nullptr },  // Inst #377 = EBREAK
  { 378,	0,	0,	4,	8,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x3ULL, nullptr, nullptr, nullptr, -1 ,nullptr },  // Inst #378 = ECALL
  { 379,	4,	1,	4,	26,	0, 0x1ULL, nullptr, nullptr, OperandInfo88, -1 ,nullptr },  // Inst #379 = FADD_D
  { 380,	4,	1,	4,	27,	0, 0x1ULL, nullptr, nullptr, OperandInfo89, -1 ,nullptr },  // Inst #380 = FADD_S
  { 381,	2,	1,	4,	28,	0, 0x1ULL, nullptr, nullptr, OperandInfo90, -1 ,nullptr },  // Inst #381 = FCLASS_D
  { 382,	2,	1,	4,	29,	0, 0x1ULL, nullptr, nullptr, OperandInfo91, -1 ,nullptr },  // Inst #382 = FCLASS_S
  { 383,	3,	1,	4,	30,	0, 0x1ULL, nullptr, nullptr, OperandInfo46, -1 ,nullptr },  // Inst #383 = FCVT_D_L
  { 384,	3,	1,	4,	30,	0, 0x1ULL, nullptr, nullptr, OperandInfo46, -1 ,nullptr },  // Inst #384 = FCVT_D_LU
  { 385,	2,	1,	4,	31,	0, 0x1ULL, nullptr, nullptr, OperandInfo92, -1 ,nullptr },  // Inst #385 = FCVT_D_S
  { 386,	2,	1,	4,	32,	0, 0x1ULL, nullptr, nullptr, OperandInfo93, -1 ,nullptr },  // Inst #386 = FCVT_D_W
  { 387,	2,	1,	4,	32,	0, 0x1ULL, nullptr, nullptr, OperandInfo93, -1 ,nullptr },  // Inst #387 = FCVT_D_WU
  { 388,	3,	1,	4,	33,	0, 0x1ULL, nullptr, nullptr, OperandInfo94, -1 ,nullptr },  // Inst #388 = FCVT_LU_D
  { 389,	3,	1,	4,	34,	0, 0x1ULL, nullptr, nullptr, OperandInfo95, -1 ,nullptr },  // Inst #389 = FCVT_LU_S
  { 390,	3,	1,	4,	33,	0, 0x1ULL, nullptr, nullptr, OperandInfo94, -1 ,nullptr },  // Inst #390 = FCVT_L_D
  { 391,	3,	1,	4,	34,	0, 0x1ULL, nullptr, nullptr, OperandInfo95, -1 ,nullptr },  // Inst #391 = FCVT_L_S
  { 392,	3,	1,	4,	35,	0, 0x1ULL, nullptr, nullptr, OperandInfo96, -1 ,nullptr },  // Inst #392 = FCVT_S_D
  { 393,	3,	1,	4,	36,	0, 0x1ULL, nullptr, nullptr, OperandInfo47, -1 ,nullptr },  // Inst #393 = FCVT_S_L
  { 394,	3,	1,	4,	36,	0, 0x1ULL, nullptr, nullptr, OperandInfo47, -1 ,nullptr },  // Inst #394 = FCVT_S_LU
  { 395,	3,	1,	4,	37,	0, 0x1ULL, nullptr, nullptr, OperandInfo47, -1 ,nullptr },  // Inst #395 = FCVT_S_W
  { 396,	3,	1,	4,	37,	0, 0x1ULL, nullptr, nullptr, OperandInfo47, -1 ,nullptr },  // Inst #396 = FCVT_S_WU
  { 397,	3,	1,	4,	38,	0, 0x1ULL, nullptr, nullptr, OperandInfo94, -1 ,nullptr },  // Inst #397 = FCVT_WU_D
  { 398,	3,	1,	4,	39,	0, 0x1ULL, nullptr, nullptr, OperandInfo95, -1 ,nullptr },  // Inst #398 = FCVT_WU_S
  { 399,	3,	1,	4,	38,	0, 0x1ULL, nullptr, nullptr, OperandInfo94, -1 ,nullptr },  // Inst #399 = FCVT_W_D
  { 400,	3,	1,	4,	39,	0, 0x1ULL, nullptr, nullptr, OperandInfo95, -1 ,nullptr },  // Inst #400 = FCVT_W_S
  { 401,	4,	1,	4,	26,	0, 0x1ULL, nullptr, nullptr, OperandInfo88, -1 ,nullptr },  // Inst #401 = FDIV_D
  { 402,	4,	1,	4,	40,	0, 0x1ULL, nullptr, nullptr, OperandInfo89, -1 ,nullptr },  // Inst #402 = FDIV_S
  { 403,	2,	0,	4,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x3ULL, nullptr, nullptr, OperandInfo97, -1 ,nullptr },  // Inst #403 = FENCE
  { 404,	0,	0,	4,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x3ULL, nullptr, nullptr, nullptr, -1 ,nullptr },  // Inst #404 = FENCE_I
  { 405,	0,	0,	4,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x3ULL, nullptr, nullptr, nullptr, -1 ,nullptr },  // Inst #405 = FENCE_TSO
  { 406,	3,	1,	4,	41,	0, 0x1ULL, nullptr, nullptr, OperandInfo98, -1 ,nullptr },  // Inst #406 = FEQ_D
  { 407,	3,	1,	4,	42,	0, 0x1ULL, nullptr, nullptr, OperandInfo99, -1 ,nullptr },  // Inst #407 = FEQ_S
  { 408,	3,	1,	4,	11,	0|(1ULL<<MCID::MayLoad), 0x3ULL, nullptr, nullptr, OperandInfo100, -1 ,nullptr },  // Inst #408 = FLD
  { 409,	3,	1,	4,	41,	0, 0x1ULL, nullptr, nullptr, OperandInfo98, -1 ,nullptr },  // Inst #409 = FLE_D
  { 410,	3,	1,	4,	42,	0, 0x1ULL, nullptr, nullptr, OperandInfo99, -1 ,nullptr },  // Inst #410 = FLE_S
  { 411,	3,	1,	4,	41,	0, 0x1ULL, nullptr, nullptr, OperandInfo98, -1 ,nullptr },  // Inst #411 = FLT_D
  { 412,	3,	1,	4,	42,	0, 0x1ULL, nullptr, nullptr, OperandInfo99, -1 ,nullptr },  // Inst #412 = FLT_S
  { 413,	3,	1,	4,	12,	0|(1ULL<<MCID::MayLoad), 0x3ULL, nullptr, nullptr, OperandInfo101, -1 ,nullptr },  // Inst #413 = FLW
  { 414,	5,	1,	4,	43,	0, 0x2ULL, nullptr, nullptr, OperandInfo102, -1 ,nullptr },  // Inst #414 = FMADD_D
  { 415,	5,	1,	4,	44,	0, 0x2ULL, nullptr, nullptr, OperandInfo103, -1 ,nullptr },  // Inst #415 = FMADD_S
  { 416,	3,	1,	4,	26,	0, 0x1ULL, nullptr, nullptr, OperandInfo104, -1 ,nullptr },  // Inst #416 = FMAX_D
  { 417,	3,	1,	4,	27,	0, 0x1ULL, nullptr, nullptr, OperandInfo105, -1 ,nullptr },  // Inst #417 = FMAX_S
  { 418,	3,	1,	4,	26,	0, 0x1ULL, nullptr, nullptr, OperandInfo104, -1 ,nullptr },  // Inst #418 = FMIN_D
  { 419,	3,	1,	4,	27,	0, 0x1ULL, nullptr, nullptr, OperandInfo105, -1 ,nullptr },  // Inst #419 = FMIN_S
  { 420,	5,	1,	4,	45,	0, 0x2ULL, nullptr, nullptr, OperandInfo102, -1 ,nullptr },  // Inst #420 = FMSUB_D
  { 421,	5,	1,	4,	46,	0, 0x2ULL, nullptr, nullptr, OperandInfo103, -1 ,nullptr },  // Inst #421 = FMSUB_S
  { 422,	4,	1,	4,	26,	0, 0x1ULL, nullptr, nullptr, OperandInfo88, -1 ,nullptr },  // Inst #422 = FMUL_D
  { 423,	4,	1,	4,	47,	0, 0x1ULL, nullptr, nullptr, OperandInfo89, -1 ,nullptr },  // Inst #423 = FMUL_S
  { 424,	2,	1,	4,	48,	0, 0x1ULL, nullptr, nullptr, OperandInfo93, -1 ,nullptr },  // Inst #424 = FMV_D_X
  { 425,	2,	1,	4,	49,	0, 0x1ULL, nullptr, nullptr, OperandInfo106, -1 ,nullptr },  // Inst #425 = FMV_W_X
  { 426,	2,	1,	4,	50,	0, 0x1ULL, nullptr, nullptr, OperandInfo90, -1 ,nullptr },  // Inst #426 = FMV_X_D
  { 427,	2,	1,	4,	51,	0, 0x1ULL, nullptr, nullptr, OperandInfo91, -1 ,nullptr },  // Inst #427 = FMV_X_W
  { 428,	5,	1,	4,	43,	0, 0x2ULL, nullptr, nullptr, OperandInfo102, -1 ,nullptr },  // Inst #428 = FNMADD_D
  { 429,	5,	1,	4,	44,	0, 0x2ULL, nullptr, nullptr, OperandInfo103, -1 ,nullptr },  // Inst #429 = FNMADD_S
  { 430,	5,	1,	4,	45,	0, 0x2ULL, nullptr, nullptr, OperandInfo102, -1 ,nullptr },  // Inst #430 = FNMSUB_D
  { 431,	5,	1,	4,	46,	0, 0x2ULL, nullptr, nullptr, OperandInfo103, -1 ,nullptr },  // Inst #431 = FNMSUB_S
  { 432,	3,	0,	4,	13,	0|(1ULL<<MCID::MayStore), 0x4ULL, nullptr, nullptr, OperandInfo100, -1 ,nullptr },  // Inst #432 = FSD
  { 433,	3,	1,	4,	26,	0, 0x1ULL, nullptr, nullptr, OperandInfo104, -1 ,nullptr },  // Inst #433 = FSGNJN_D
  { 434,	3,	1,	4,	27,	0, 0x1ULL, nullptr, nullptr, OperandInfo105, -1 ,nullptr },  // Inst #434 = FSGNJN_S
  { 435,	3,	1,	4,	26,	0, 0x1ULL, nullptr, nullptr, OperandInfo104, -1 ,nullptr },  // Inst #435 = FSGNJX_D
  { 436,	3,	1,	4,	27,	0, 0x1ULL, nullptr, nullptr, OperandInfo105, -1 ,nullptr },  // Inst #436 = FSGNJX_S
  { 437,	3,	1,	4,	26,	0, 0x1ULL, nullptr, nullptr, OperandInfo104, -1 ,nullptr },  // Inst #437 = FSGNJ_D
  { 438,	3,	1,	4,	27,	0, 0x1ULL, nullptr, nullptr, OperandInfo105, -1 ,nullptr },  // Inst #438 = FSGNJ_S
  { 439,	3,	1,	4,	52,	0, 0x1ULL, nullptr, nullptr, OperandInfo107, -1 ,nullptr },  // Inst #439 = FSQRT_D
  { 440,	3,	1,	4,	52,	0, 0x1ULL, nullptr, nullptr, OperandInfo108, -1 ,nullptr },  // Inst #440 = FSQRT_S
  { 441,	4,	1,	4,	26,	0, 0x1ULL, nullptr, nullptr, OperandInfo88, -1 ,nullptr },  // Inst #441 = FSUB_D
  { 442,	4,	1,	4,	27,	0, 0x1ULL, nullptr, nullptr, OperandInfo89, -1 ,nullptr },  // Inst #442 = FSUB_S
  { 443,	3,	0,	4,	14,	0|(1ULL<<MCID::MayStore), 0x4ULL, nullptr, nullptr, OperandInfo101, -1 ,nullptr },  // Inst #443 = FSW
  { 444,	2,	1,	4,	15,	0|(1ULL<<MCID::Call), 0x7ULL, nullptr, nullptr, OperandInfo109, -1 ,nullptr },  // Inst #444 = JAL
  { 445,	3,	1,	4,	16,	0|(1ULL<<MCID::Call), 0x3ULL, nullptr, nullptr, OperandInfo59, -1 ,nullptr },  // Inst #445 = JALR
  { 446,	3,	1,	4,	53,	0|(1ULL<<MCID::MayLoad), 0x3ULL, nullptr, nullptr, OperandInfo59, -1 ,nullptr },  // Inst #446 = LB
  { 447,	3,	1,	4,	53,	0|(1ULL<<MCID::MayLoad), 0x3ULL, nullptr, nullptr, OperandInfo59, -1 ,nullptr },  // Inst #447 = LBU
  { 448,	3,	1,	4,	18,	0|(1ULL<<MCID::MayLoad), 0x3ULL, nullptr, nullptr, OperandInfo59, -1 ,nullptr },  // Inst #448 = LD
  { 449,	3,	1,	4,	54,	0|(1ULL<<MCID::MayLoad), 0x3ULL, nullptr, nullptr, OperandInfo59, -1 ,nullptr },  // Inst #449 = LH
  { 450,	3,	1,	4,	54,	0|(1ULL<<MCID::MayLoad), 0x3ULL, nullptr, nullptr, OperandInfo59, -1 ,nullptr },  // Inst #450 = LHU
  { 451,	2,	1,	4,	55,	0|(1ULL<<MCID::MayLoad), 0x1ULL, nullptr, nullptr, OperandInfo53, -1 ,nullptr },  // Inst #451 = LR_D
  { 452,	2,	1,	4,	55,	0|(1ULL<<MCID::MayLoad), 0x1ULL, nullptr, nullptr, OperandInfo53, -1 ,nullptr },  // Inst #452 = LR_D_AQ
  { 453,	2,	1,	4,	55,	0|(1ULL<<MCID::MayLoad), 0x1ULL, nullptr, nullptr, OperandInfo53, -1 ,nullptr },  // Inst #453 = LR_D_AQ_RL
  { 454,	2,	1,	4,	55,	0|(1ULL<<MCID::MayLoad), 0x1ULL, nullptr, nullptr, OperandInfo53, -1 ,nullptr },  // Inst #454 = LR_D_RL
  { 455,	2,	1,	4,	56,	0|(1ULL<<MCID::MayLoad), 0x1ULL, nullptr, nullptr, OperandInfo53, -1 ,nullptr },  // Inst #455 = LR_W
  { 456,	2,	1,	4,	56,	0|(1ULL<<MCID::MayLoad), 0x1ULL, nullptr, nullptr, OperandInfo53, -1 ,nullptr },  // Inst #456 = LR_W_AQ
  { 457,	2,	1,	4,	56,	0|(1ULL<<MCID::MayLoad), 0x1ULL, nullptr, nullptr, OperandInfo53, -1 ,nullptr },  // Inst #457 = LR_W_AQ_RL
  { 458,	2,	1,	4,	56,	0|(1ULL<<MCID::MayLoad), 0x1ULL, nullptr, nullptr, OperandInfo53, -1 ,nullptr },  // Inst #458 = LR_W_RL
  { 459,	2,	1,	4,	7,	0|(1ULL<<MCID::Rematerializable)|(1ULL<<MCID::CheapAsAMove), 0x6ULL, nullptr, nullptr, OperandInfo60, -1 ,nullptr },  // Inst #459 = LUI
  { 460,	3,	1,	4,	19,	0|(1ULL<<MCID::MayLoad), 0x3ULL, nullptr, nullptr, OperandInfo59, -1 ,nullptr },  // Inst #460 = LW
  { 461,	3,	1,	4,	57,	0|(1ULL<<MCID::MayLoad), 0x3ULL, nullptr, nullptr, OperandInfo59, -1 ,nullptr },  // Inst #461 = LWU
  { 462,	2,	0,	4,	0,	0|(1ULL<<MCID::Return)|(1ULL<<MCID::Barrier)|(1ULL<<MCID::Terminator)|(1ULL<<MCID::UnmodeledSideEffects), 0x1ULL, nullptr, nullptr, OperandInfo53, -1 ,nullptr },  // Inst #462 = MRET
  { 463,	3,	1,	4,	58,	0, 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #463 = MUL
  { 464,	3,	1,	4,	58,	0, 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #464 = MULH
  { 465,	3,	1,	4,	58,	0, 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #465 = MULHSU
  { 466,	3,	1,	4,	58,	0, 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #466 = MULHU
  { 467,	3,	1,	4,	59,	0, 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #467 = MULW
  { 468,	3,	1,	4,	1,	0, 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #468 = OR
  { 469,	3,	1,	4,	2,	0|(1ULL<<MCID::Rematerializable)|(1ULL<<MCID::CheapAsAMove), 0x3ULL, nullptr, nullptr, OperandInfo59, -1 ,nullptr },  // Inst #469 = ORI
  { 470,	3,	1,	4,	24,	0, 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #470 = REM
  { 471,	3,	1,	4,	24,	0, 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #471 = REMU
  { 472,	3,	1,	4,	25,	0, 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #472 = REMUW
  { 473,	3,	1,	4,	25,	0, 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #473 = REMW
  { 474,	3,	0,	4,	60,	0|(1ULL<<MCID::MayStore), 0x4ULL, nullptr, nullptr, OperandInfo59, -1 ,nullptr },  // Inst #474 = SB
  { 475,	3,	1,	4,	61,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #475 = SC_D
  { 476,	3,	1,	4,	61,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #476 = SC_D_AQ
  { 477,	3,	1,	4,	61,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #477 = SC_D_AQ_RL
  { 478,	3,	1,	4,	61,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #478 = SC_D_RL
  { 479,	3,	1,	4,	62,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #479 = SC_W
  { 480,	3,	1,	4,	62,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #480 = SC_W_AQ
  { 481,	3,	1,	4,	62,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #481 = SC_W_AQ_RL
  { 482,	3,	1,	4,	62,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #482 = SC_W_RL
  { 483,	3,	0,	4,	21,	0|(1ULL<<MCID::MayStore), 0x4ULL, nullptr, nullptr, OperandInfo59, -1 ,nullptr },  // Inst #483 = SD
  { 484,	2,	0,	4,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x1ULL, nullptr, nullptr, OperandInfo53, -1 ,nullptr },  // Inst #484 = SFENCE_VMA
  { 485,	3,	0,	4,	63,	0|(1ULL<<MCID::MayStore), 0x4ULL, nullptr, nullptr, OperandInfo59, -1 ,nullptr },  // Inst #485 = SH
  { 486,	3,	1,	4,	1,	0, 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #486 = SLL
  { 487,	3,	1,	4,	22,	0, 0x3ULL, nullptr, nullptr, OperandInfo110, -1 ,nullptr },  // Inst #487 = SLLI
  { 488,	3,	1,	4,	64,	0, 0x3ULL, nullptr, nullptr, OperandInfo111, -1 ,nullptr },  // Inst #488 = SLLIW
  { 489,	3,	1,	4,	4,	0, 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #489 = SLLW
  { 490,	3,	1,	4,	1,	0, 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #490 = SLT
  { 491,	3,	1,	4,	2,	0, 0x3ULL, nullptr, nullptr, OperandInfo59, -1 ,nullptr },  // Inst #491 = SLTI
  { 492,	3,	1,	4,	2,	0, 0x3ULL, nullptr, nullptr, OperandInfo59, -1 ,nullptr },  // Inst #492 = SLTIU
  { 493,	3,	1,	4,	1,	0, 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #493 = SLTU
  { 494,	3,	1,	4,	1,	0, 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #494 = SRA
  { 495,	3,	1,	4,	22,	0, 0x3ULL, nullptr, nullptr, OperandInfo110, -1 ,nullptr },  // Inst #495 = SRAI
  { 496,	3,	1,	4,	64,	0, 0x3ULL, nullptr, nullptr, OperandInfo111, -1 ,nullptr },  // Inst #496 = SRAIW
  { 497,	3,	1,	4,	4,	0, 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #497 = SRAW
  { 498,	2,	0,	4,	0,	0|(1ULL<<MCID::Return)|(1ULL<<MCID::Barrier)|(1ULL<<MCID::Terminator)|(1ULL<<MCID::UnmodeledSideEffects), 0x1ULL, nullptr, nullptr, OperandInfo53, -1 ,nullptr },  // Inst #498 = SRET
  { 499,	3,	1,	4,	1,	0, 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #499 = SRL
  { 500,	3,	1,	4,	22,	0, 0x3ULL, nullptr, nullptr, OperandInfo110, -1 ,nullptr },  // Inst #500 = SRLI
  { 501,	3,	1,	4,	64,	0, 0x3ULL, nullptr, nullptr, OperandInfo111, -1 ,nullptr },  // Inst #501 = SRLIW
  { 502,	3,	1,	4,	4,	0, 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #502 = SRLW
  { 503,	3,	1,	4,	1,	0, 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #503 = SUB
  { 504,	3,	1,	4,	4,	0, 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #504 = SUBW
  { 505,	3,	0,	4,	23,	0|(1ULL<<MCID::MayStore), 0x4ULL, nullptr, nullptr, OperandInfo59, -1 ,nullptr },  // Inst #505 = SW
  { 506,	0,	0,	4,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x3ULL, nullptr, nullptr, nullptr, -1 ,nullptr },  // Inst #506 = UNIMP
  { 507,	2,	0,	4,	0,	0|(1ULL<<MCID::Return)|(1ULL<<MCID::Barrier)|(1ULL<<MCID::Terminator)|(1ULL<<MCID::UnmodeledSideEffects), 0x1ULL, nullptr, nullptr, OperandInfo53, -1 ,nullptr },  // Inst #507 = URET
  { 508,	2,	0,	4,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x1ULL, nullptr, nullptr, OperandInfo53, -1 ,nullptr },  // Inst #508 = WFI
  { 509,	3,	1,	4,	1,	0, 0x1ULL, nullptr, nullptr, OperandInfo58, -1 ,nullptr },  // Inst #509 = XOR
  { 510,	3,	1,	4,	2,	0|(1ULL<<MCID::Rematerializable)|(1ULL<<MCID::CheapAsAMove), 0x3ULL, nullptr, nullptr, OperandInfo59, -1 ,nullptr },  // Inst #510 = XORI
};

extern const char RISCVInstrNameData[] = {
  /* 0 */ 'G', '_', 'F', 'L', 'O', 'G', '1', '0', 0,
  /* 9 */ 'C', '_', 'A', 'D', 'D', 'I', '_', 'H', 'I', 'N', 'T', '_', 'X', '0', 0,
  /* 24 */ 'P', 's', 'e', 'u', 'd', 'o', 'M', 'a', 's', 'k', 'e', 'd', 'A', 't', 'o', 'm', 'i', 'c', 'L', 'o', 'a', 'd', 'S', 'u', 'b', '3', '2', 0,
  /* 52 */ 'P', 's', 'e', 'u', 'd', 'o', 'M', 'a', 's', 'k', 'e', 'd', 'A', 't', 'o', 'm', 'i', 'c', 'L', 'o', 'a', 'd', 'A', 'd', 'd', '3', '2', 0,
  /* 80 */ 'P', 's', 'e', 'u', 'd', 'o', 'M', 'a', 's', 'k', 'e', 'd', 'A', 't', 'o', 'm', 'i', 'c', 'L', 'o', 'a', 'd', 'N', 'a', 'n', 'd', '3', '2', 0,
  /* 109 */ 'P', 's', 'e', 'u', 'd', 'o', 'A', 't', 'o', 'm', 'i', 'c', 'L', 'o', 'a', 'd', 'N', 'a', 'n', 'd', '3', '2', 0,
  /* 132 */ 'P', 's', 'e', 'u', 'd', 'o', 'M', 'a', 's', 'k', 'e', 'd', 'C', 'm', 'p', 'X', 'c', 'h', 'g', '3', '2', 0,
  /* 154 */ 'P', 's', 'e', 'u', 'd', 'o', 'C', 'm', 'p', 'X', 'c', 'h', 'g', '3', '2', 0,
  /* 170 */ 'P', 's', 'e', 'u', 'd', 'o', 'M', 'a', 's', 'k', 'e', 'd', 'A', 't', 'o', 'm', 'i', 'c', 'L', 'o', 'a', 'd', 'U', 'M', 'i', 'n', '3', '2', 0,
  /* 199 */ 'P', 's', 'e', 'u', 'd', 'o', 'M', 'a', 's', 'k', 'e', 'd', 'A', 't', 'o', 'm', 'i', 'c', 'L', 'o', 'a', 'd', 'M', 'i', 'n', '3', '2', 0,
  /* 227 */ 'P', 's', 'e', 'u', 'd', 'o', 'M', 'a', 's', 'k', 'e', 'd', 'A', 't', 'o', 'm', 'i', 'c', 'S', 'w', 'a', 'p', '3', '2', 0,
  /* 252 */ 'P', 's', 'e', 'u', 'd', 'o', 'M', 'a', 's', 'k', 'e', 'd', 'A', 't', 'o', 'm', 'i', 'c', 'L', 'o', 'a', 'd', 'U', 'M', 'a', 'x', '3', '2', 0,
  /* 281 */ 'P', 's', 'e', 'u', 'd', 'o', 'M', 'a', 's', 'k', 'e', 'd', 'A', 't', 'o', 'm', 'i', 'c', 'L', 'o', 'a', 'd', 'M', 'a', 'x', '3', '2', 0,
  /* 309 */ 'G', '_', 'F', 'L', 'O', 'G', '2', 0,
  /* 317 */ 'G', '_', 'F', 'E', 'X', 'P', '2', 0,
  /* 325 */ 'P', 's', 'e', 'u', 'd', 'o', 'A', 't', 'o', 'm', 'i', 'c', 'L', 'o', 'a', 'd', 'N', 'a', 'n', 'd', '6', '4', 0,
  /* 348 */ 'P', 's', 'e', 'u', 'd', 'o', 'C', 'm', 'p', 'X', 'c', 'h', 'g', '6', '4', 0,
  /* 364 */ 'P', 's', 'e', 'u', 'd', 'o', 'L', 'L', 'A', 0,
  /* 374 */ 'P', 's', 'e', 'u', 'd', 'o', 'L', 'A', 0,
  /* 383 */ 'G', '_', 'F', 'M', 'A', 0,
  /* 389 */ 'S', 'F', 'E', 'N', 'C', 'E', '_', 'V', 'M', 'A', 0,
  /* 400 */ 'S', 'R', 'A', 0,
  /* 404 */ 'P', 's', 'e', 'u', 'd', 'o', 'L', 'B', 0,
  /* 413 */ 'P', 's', 'e', 'u', 'd', 'o', 'S', 'B', 0,
  /* 422 */ 'G', '_', 'F', 'S', 'U', 'B', 0,
  /* 429 */ 'G', '_', 'A', 'T', 'O', 'M', 'I', 'C', 'R', 'M', 'W', '_', 'F', 'S', 'U', 'B', 0,
  /* 446 */ 'C', '_', 'S', 'U', 'B', 0,
  /* 452 */ 'G', '_', 'S', 'U', 'B', 0,
  /* 458 */ 'G', '_', 'A', 'T', 'O', 'M', 'I', 'C', 'R', 'M', 'W', '_', 'S', 'U', 'B', 0,
  /* 474 */ 'G', '_', 'I', 'N', 'T', 'R', 'I', 'N', 'S', 'I', 'C', 0,
  /* 486 */ 'G', '_', 'F', 'P', 'T', 'R', 'U', 'N', 'C', 0,
  /* 496 */ 'G', '_', 'I', 'N', 'T', 'R', 'I', 'N', 'S', 'I', 'C', '_', 'T', 'R', 'U', 'N', 'C', 0,
  /* 514 */ 'G', '_', 'T', 'R', 'U', 'N', 'C', 0,
  /* 522 */ 'G', '_', 'B', 'U', 'I', 'L', 'D', '_', 'V', 'E', 'C', 'T', 'O', 'R', '_', 'T', 'R', 'U', 'N', 'C', 0,
  /* 543 */ 'G', '_', 'D', 'Y', 'N', '_', 'S', 'T', 'A', 'C', 'K', 'A', 'L', 'L', 'O', 'C', 0,
  /* 560 */ 'A', 'U', 'I', 'P', 'C', 0,
  /* 566 */ 'C', 'S', 'R', 'R', 'C', 0,
  /* 572 */ 'G', '_', 'F', 'M', 'A', 'D', 0,
  /* 579 */ 'G', '_', 'I', 'N', 'D', 'E', 'X', 'E', 'D', '_', 'S', 'E', 'X', 'T', 'L', 'O', 'A', 'D', 0,
  /* 598 */ 'G', '_', 'S', 'E', 'X', 'T', 'L', 'O', 'A', 'D', 0,
  /* 609 */ 'G', '_', 'I', 'N', 'D', 'E', 'X', 'E', 'D', '_', 'Z', 'E', 'X', 'T', 'L', 'O', 'A', 'D', 0,
  /* 628 */ 'G', '_', 'Z', 'E', 'X', 'T', 'L', 'O', 'A', 'D', 0,
  /* 639 */ 'G', '_', 'I', 'N', 'D', 'E', 'X', 'E', 'D', '_', 'L', 'O', 'A', 'D', 0,
  /* 654 */ 'G', '_', 'L', 'O', 'A', 'D', 0,
  /* 661 */ 'G', '_', 'F', 'A', 'D', 'D', 0,
  /* 668 */ 'G', '_', 'A', 'T', 'O', 'M', 'I', 'C', 'R', 'M', 'W', '_', 'F', 'A', 'D', 'D', 0,
  /* 685 */ 'C', '_', 'A', 'D', 'D', 0,
  /* 691 */ 'G', '_', 'A', 'D', 'D', 0,
  /* 697 */ 'G', '_', 'P', 'T', 'R', '_', 'A', 'D', 'D', 0,
  /* 707 */ 'G', '_', 'A', 'T', 'O', 'M', 'I', 'C', 'R', 'M', 'W', '_', 'A', 'D', 'D', 0,
  /* 723 */ 'P', 's', 'e', 'u', 'd', 'o', 'L', 'A', '_', 'T', 'L', 'S', '_', 'G', 'D', 0,
  /* 739 */ 'C', '_', 'F', 'L', 'D', 0,
  /* 745 */ 'P', 's', 'e', 'u', 'd', 'o', 'F', 'L', 'D', 0,
  /* 755 */ 'C', '_', 'L', 'D', 0,
  /* 760 */ 'P', 's', 'e', 'u', 'd', 'o', 'L', 'D', 0,
  /* 769 */ 'G', '_', 'A', 'T', 'O', 'M', 'I', 'C', 'R', 'M', 'W', '_', 'N', 'A', 'N', 'D', 0,
  /* 786 */ 'C', '_', 'A', 'N', 'D', 0,
  /* 792 */ 'G', '_', 'A', 'N', 'D', 0,
  /* 798 */ 'G', '_', 'A', 'T', 'O', 'M', 'I', 'C', 'R', 'M', 'W', '_', 'A', 'N', 'D', 0,
  /* 814 */ 'L', 'I', 'F', 'E', 'T', 'I', 'M', 'E', '_', 'E', 'N', 'D', 0,
  /* 827 */ 'P', 's', 'e', 'u', 'd', 'o', 'B', 'R', 'I', 'N', 'D', 0,
  /* 839 */ 'G', '_', 'B', 'R', 'C', 'O', 'N', 'D', 0,
  /* 848 */ 'G', '_', 'I', 'N', 'T', 'R', 'I', 'N', 'S', 'I', 'C', '_', 'R', 'O', 'U', 'N', 'D', 0,
  /* 866 */ 'L', 'O', 'A', 'D', '_', 'S', 'T', 'A', 'C', 'K', '_', 'G', 'U', 'A', 'R', 'D', 0,
  /* 883 */ 'C', '_', 'F', 'S', 'D', 0,
  /* 889 */ 'P', 's', 'e', 'u', 'd', 'o', 'F', 'S', 'D', 0,
  /* 899 */ 'C', '_', 'S', 'D', 0,
  /* 904 */ 'P', 's', 'e', 'u', 'd', 'o', 'S', 'D', 0,
  /* 913 */ 'F', 'S', 'U', 'B', '_', 'D', 0,
  /* 920 */ 'F', 'M', 'S', 'U', 'B', '_', 'D', 0,
  /* 928 */ 'F', 'N', 'M', 'S', 'U', 'B', '_', 'D', 0,
  /* 937 */ 'S', 'C', '_', 'D', 0,
  /* 942 */ 'F', 'A', 'D', 'D', '_', 'D', 0,
  /* 949 */ 'F', 'M', 'A', 'D', 'D', '_', 'D', 0,
  /* 957 */ 'F', 'N', 'M', 'A', 'D', 'D', '_', 'D', 0,
  /* 966 */ 'A', 'M', 'O', 'A', 'D', 'D', '_', 'D', 0,
  /* 975 */ 'A', 'M', 'O', 'A', 'N', 'D', '_', 'D', 0,
  /* 984 */ 'F', 'L', 'E', '_', 'D', 0,
  /* 990 */ 'F', 'S', 'G', 'N', 'J', '_', 'D', 0,
  /* 998 */ 'F', 'M', 'U', 'L', '_', 'D', 0,
  /* 1005 */ 'F', 'C', 'V', 'T', '_', 'L', '_', 'D', 0,
  /* 1014 */ 'F', 'M', 'I', 'N', '_', 'D', 0,
  /* 1021 */ 'A', 'M', 'O', 'M', 'I', 'N', '_', 'D', 0,
  /* 1030 */ 'F', 'S', 'G', 'N', 'J', 'N', '_', 'D', 0,
  /* 1039 */ 'A', 'M', 'O', 'S', 'W', 'A', 'P', '_', 'D', 0,
  /* 1049 */ 'F', 'E', 'Q', '_', 'D', 0,
  /* 1055 */ 'L', 'R', '_', 'D', 0,
  /* 1060 */ 'A', 'M', 'O', 'O', 'R', '_', 'D', 0,
  /* 1068 */ 'A', 'M', 'O', 'X', 'O', 'R', '_', 'D', 0,
  /* 1077 */ 'F', 'C', 'L', 'A', 'S', 'S', '_', 'D', 0,
  /* 1086 */ 'F', 'C', 'V', 'T', '_', 'S', '_', 'D', 0,
  /* 1095 */ 'F', 'L', 'T', '_', 'D', 0,
  /* 1101 */ 'F', 'S', 'Q', 'R', 'T', '_', 'D', 0,
  /* 1109 */ 'F', 'C', 'V', 'T', '_', 'L', 'U', '_', 'D', 0,
  /* 1119 */ 'A', 'M', 'O', 'M', 'I', 'N', 'U', '_', 'D', 0,
  /* 1129 */ 'F', 'C', 'V', 'T', '_', 'W', 'U', '_', 'D', 0,
  /* 1139 */ 'A', 'M', 'O', 'M', 'A', 'X', 'U', '_', 'D', 0,
  /* 1149 */ 'F', 'D', 'I', 'V', '_', 'D', 0,
  /* 1156 */ 'F', 'C', 'V', 'T', '_', 'W', '_', 'D', 0,
  /* 1165 */ 'F', 'M', 'A', 'X', '_', 'D', 0,
  /* 1172 */ 'A', 'M', 'O', 'M', 'A', 'X', '_', 'D', 0,
  /* 1181 */ 'F', 'S', 'G', 'N', 'J', 'X', '_', 'D', 0,
  /* 1190 */ 'F', 'M', 'V', '_', 'X', '_', 'D', 0,
  /* 1198 */ 'G', '_', 'S', 'S', 'U', 'B', 'E', 0,
  /* 1206 */ 'G', '_', 'U', 'S', 'U', 'B', 'E', 0,
  /* 1214 */ 'G', '_', 'F', 'E', 'N', 'C', 'E', 0,
  /* 1222 */ 'R', 'E', 'G', '_', 'S', 'E', 'Q', 'U', 'E', 'N', 'C', 'E', 0,
  /* 1235 */ 'G', '_', 'S', 'A', 'D', 'D', 'E', 0,
  /* 1243 */ 'G', '_', 'U', 'A', 'D', 'D', 'E', 0,
  /* 1251 */ 'G', '_', 'F', 'M', 'I', 'N', 'N', 'U', 'M', '_', 'I', 'E', 'E', 'E', 0,
  /* 1266 */ 'G', '_', 'F', 'M', 'A', 'X', 'N', 'U', 'M', '_', 'I', 'E', 'E', 'E', 0,
  /* 1281 */ 'B', 'G', 'E', 0,
  /* 1285 */ 'P', 's', 'e', 'u', 'd', 'o', 'L', 'A', '_', 'T', 'L', 'S', '_', 'I', 'E', 0,
  /* 1301 */ 'G', '_', 'J', 'U', 'M', 'P', '_', 'T', 'A', 'B', 'L', 'E', 0,
  /* 1314 */ 'B', 'U', 'N', 'D', 'L', 'E', 0,
  /* 1321 */ 'B', 'N', 'E', 0,
  /* 1325 */ 'L', 'O', 'C', 'A', 'L', '_', 'E', 'S', 'C', 'A', 'P', 'E', 0,
  /* 1338 */ 'G', '_', 'I', 'N', 'D', 'E', 'X', 'E', 'D', '_', 'S', 'T', 'O', 'R', 'E', 0,
  /* 1354 */ 'G', '_', 'S', 'T', 'O', 'R', 'E', 0,
  /* 1362 */ 'G', '_', 'B', 'I', 'T', 'R', 'E', 'V', 'E', 'R', 'S', 'E', 0,
  /* 1375 */ 'D', 'B', 'G', '_', 'V', 'A', 'L', 'U', 'E', 0,
  /* 1385 */ 'G', '_', 'G', 'L', 'O', 'B', 'A', 'L', '_', 'V', 'A', 'L', 'U', 'E', 0,
  /* 1400 */ 'G', '_', 'F', 'C', 'A', 'N', 'O', 'N', 'I', 'C', 'A', 'L', 'I', 'Z', 'E', 0,
  /* 1416 */ 'G', '_', 'C', 'T', 'L', 'Z', '_', 'Z', 'E', 'R', 'O', '_', 'U', 'N', 'D', 'E', 'F', 0,
  /* 1434 */ 'G', '_', 'C', 'T', 'T', 'Z', '_', 'Z', 'E', 'R', 'O', '_', 'U', 'N', 'D', 'E', 'F', 0,
  /* 1452 */ 'G', '_', 'I', 'M', 'P', 'L', 'I', 'C', 'I', 'T', '_', 'D', 'E', 'F', 0,
  /* 1467 */ 'G', '_', 'F', 'N', 'E', 'G', 0,
  /* 1474 */ 'E', 'X', 'T', 'R', 'A', 'C', 'T', '_', 'S', 'U', 'B', 'R', 'E', 'G', 0,
  /* 1489 */ 'I', 'N', 'S', 'E', 'R', 'T', '_', 'S', 'U', 'B', 'R', 'E', 'G', 0,
  /* 1503 */ 'G', '_', 'S', 'E', 'X', 'T', '_', 'I', 'N', 'R', 'E', 'G', 0,
  /* 1516 */ 'S', 'U', 'B', 'R', 'E', 'G', '_', 'T', 'O', '_', 'R', 'E', 'G', 0,
  /* 1530 */ 'G', '_', 'A', 'T', 'O', 'M', 'I', 'C', '_', 'C', 'M', 'P', 'X', 'C', 'H', 'G', 0,
  /* 1547 */ 'G', '_', 'A', 'T', 'O', 'M', 'I', 'C', 'R', 'M', 'W', '_', 'X', 'C', 'H', 'G', 0,
  /* 1564 */ 'G', '_', 'F', 'L', 'O', 'G', 0,
  /* 1571 */ 'G', '_', 'V', 'A', 'A', 'R', 'G', 0,
  /* 1579 */ 'G', '_', 'S', 'M', 'U', 'L', 'H', 0,
  /* 1587 */ 'G', '_', 'U', 'M', 'U', 'L', 'H', 0,
  /* 1595 */ 'P', 's', 'e', 'u', 'd', 'o', 'L', 'H', 0,
  /* 1604 */ 'P', 's', 'e', 'u', 'd', 'o', 'S', 'H', 0,
  /* 1613 */ 'C', '_', 'S', 'R', 'A', 'I', 0,
  /* 1620 */ 'C', 'S', 'R', 'R', 'C', 'I', 0,
  /* 1627 */ 'C', '_', 'A', 'D', 'D', 'I', 0,
  /* 1634 */ 'C', '_', 'A', 'N', 'D', 'I', 0,
  /* 1641 */ 'W', 'F', 'I', 0,
  /* 1645 */ 'G', '_', 'P', 'H', 'I', 0,
  /* 1651 */ 'C', '_', 'S', 'L', 'L', 'I', 0,
  /* 1658 */ 'C', '_', 'S', 'R', 'L', 'I', 0,
  /* 1665 */ 'C', '_', 'L', 'I', 0,
  /* 1670 */ 'P', 's', 'e', 'u', 'd', 'o', 'L', 'I', 0,
  /* 1679 */ 'X', 'O', 'R', 'I', 0,
  /* 1684 */ 'G', '_', 'F', 'P', 'T', 'O', 'S', 'I', 0,
  /* 1693 */ 'C', 'S', 'R', 'R', 'S', 'I', 0,
  /* 1700 */ 'S', 'L', 'T', 'I', 0,
  /* 1705 */ 'C', '_', 'L', 'U', 'I', 0,
  /* 1711 */ 'G', '_', 'F', 'P', 'T', 'O', 'U', 'I', 0,
  /* 1720 */ 'C', 'S', 'R', 'R', 'W', 'I', 0,
  /* 1727 */ 'F', 'E', 'N', 'C', 'E', '_', 'I', 0,
  /* 1735 */ 'C', '_', 'J', 0,
  /* 1739 */ 'C', '_', 'E', 'B', 'R', 'E', 'A', 'K', 0,
  /* 1748 */ 'G', '_', 'P', 'T', 'R', '_', 'M', 'A', 'S', 'K', 0,
  /* 1759 */ 'C', '_', 'J', 'A', 'L', 0,
  /* 1765 */ 'G', 'C', '_', 'L', 'A', 'B', 'E', 'L', 0,
  /* 1774 */ 'D', 'B', 'G', '_', 'L', 'A', 'B', 'E', 'L', 0,
  /* 1784 */ 'E', 'H', '_', 'L', 'A', 'B', 'E', 'L', 0,
  /* 1793 */ 'A', 'N', 'N', 'O', 'T', 'A', 'T', 'I', 'O', 'N', '_', 'L', 'A', 'B', 'E', 'L', 0,
  /* 1810 */ 'I', 'C', 'A', 'L', 'L', '_', 'B', 'R', 'A', 'N', 'C', 'H', '_', 'F', 'U', 'N', 'N', 'E', 'L', 0,
  /* 1830 */ 'G', '_', 'S', 'H', 'L', 0,
  /* 1836 */ 'P', 's', 'e', 'u', 'd', 'o', 'T', 'A', 'I', 'L', 0,
  /* 1847 */ 'G', '_', 'F', 'C', 'E', 'I', 'L', 0,
  /* 1855 */ 'E', 'C', 'A', 'L', 'L', 0,
  /* 1861 */ 'P', 'A', 'T', 'C', 'H', 'A', 'B', 'L', 'E', '_', 'T', 'A', 'I', 'L', '_', 'C', 'A', 'L', 'L', 0,
  /* 1881 */ 'P', 'A', 'T', 'C', 'H', 'A', 'B', 'L', 'E', '_', 'T', 'Y', 'P', 'E', 'D', '_', 'E', 'V', 'E', 'N', 'T', '_', 'C', 'A', 'L', 'L', 0,
  /* 1908 */ 'P', 'A', 'T', 'C', 'H', 'A', 'B', 'L', 'E', '_', 'E', 'V', 'E', 'N', 'T', '_', 'C', 'A', 'L', 'L', 0,
  /* 1929 */ 'F', 'E', 'N', 'T', 'R', 'Y', '_', 'C', 'A', 'L', 'L', 0,
  /* 1941 */ 'P', 's', 'e', 'u', 'd', 'o', 'C', 'A', 'L', 'L', 0,
  /* 1952 */ 'K', 'I', 'L', 'L', 0,
  /* 1957 */ 'S', 'L', 'L', 0,
  /* 1961 */ 'S', 'R', 'L', 0,
  /* 1965 */ 'S', 'C', '_', 'D', '_', 'R', 'L', 0,
  /* 1973 */ 'A', 'M', 'O', 'A', 'D', 'D', '_', 'D', '_', 'R', 'L', 0,
  /* 1985 */ 'A', 'M', 'O', 'A', 'N', 'D', '_', 'D', '_', 'R', 'L', 0,
  /* 1997 */ 'A', 'M', 'O', 'M', 'I', 'N', '_', 'D', '_', 'R', 'L', 0,
  /* 2009 */ 'A', 'M', 'O', 'S', 'W', 'A', 'P', '_', 'D', '_', 'R', 'L', 0,
  /* 2022 */ 'L', 'R', '_', 'D', '_', 'R', 'L', 0,
  /* 2030 */ 'A', 'M', 'O', 'O', 'R', '_', 'D', '_', 'R', 'L', 0,
  /* 2041 */ 'A', 'M', 'O', 'X', 'O', 'R', '_', 'D', '_', 'R', 'L', 0,
  /* 2053 */ 'A', 'M', 'O', 'M', 'I', 'N', 'U', '_', 'D', '_', 'R', 'L', 0,
  /* 2066 */ 'A', 'M', 'O', 'M', 'A', 'X', 'U', '_', 'D', '_', 'R', 'L', 0,
  /* 2079 */ 'A', 'M', 'O', 'M', 'A', 'X', '_', 'D', '_', 'R', 'L', 0,
  /* 2091 */ 'S', 'C', '_', 'D', '_', 'A', 'Q', '_', 'R', 'L', 0,
  /* 2102 */ 'A', 'M', 'O', 'A', 'D', 'D', '_', 'D', '_', 'A', 'Q', '_', 'R', 'L', 0,
  /* 2117 */ 'A', 'M', 'O', 'A', 'N', 'D', '_', 'D', '_', 'A', 'Q', '_', 'R', 'L', 0,
  /* 2132 */ 'A', 'M', 'O', 'M', 'I', 'N', '_', 'D', '_', 'A', 'Q', '_', 'R', 'L', 0,
  /* 2147 */ 'A', 'M', 'O', 'S', 'W', 'A', 'P', '_', 'D', '_', 'A', 'Q', '_', 'R', 'L', 0,
  /* 2163 */ 'L', 'R', '_', 'D', '_', 'A', 'Q', '_', 'R', 'L', 0,
  /* 2174 */ 'A', 'M', 'O', 'O', 'R', '_', 'D', '_', 'A', 'Q', '_', 'R', 'L', 0,
  /* 2188 */ 'A', 'M', 'O', 'X', 'O', 'R', '_', 'D', '_', 'A', 'Q', '_', 'R', 'L', 0,
  /* 2203 */ 'A', 'M', 'O', 'M', 'I', 'N', 'U', '_', 'D', '_', 'A', 'Q', '_', 'R', 'L', 0,
  /* 2219 */ 'A', 'M', 'O', 'M', 'A', 'X', 'U', '_', 'D', '_', 'A', 'Q', '_', 'R', 'L', 0,
  /* 2235 */ 'A', 'M', 'O', 'M', 'A', 'X', '_', 'D', '_', 'A', 'Q', '_', 'R', 'L', 0,
  /* 2250 */ 'S', 'C', '_', 'W', '_', 'A', 'Q', '_', 'R', 'L', 0,
  /* 2261 */ 'A', 'M', 'O', 'A', 'D', 'D', '_', 'W', '_', 'A', 'Q', '_', 'R', 'L', 0,
  /* 2276 */ 'A', 'M', 'O', 'A', 'N', 'D', '_', 'W', '_', 'A', 'Q', '_', 'R', 'L', 0,
  /* 2291 */ 'A', 'M', 'O', 'M', 'I', 'N', '_', 'W', '_', 'A', 'Q', '_', 'R', 'L', 0,
  /* 2306 */ 'A', 'M', 'O', 'S', 'W', 'A', 'P', '_', 'W', '_', 'A', 'Q', '_', 'R', 'L', 0,
  /* 2322 */ 'L', 'R', '_', 'W', '_', 'A', 'Q', '_', 'R', 'L', 0,
  /* 2333 */ 'A', 'M', 'O', 'O', 'R', '_', 'W', '_', 'A', 'Q', '_', 'R', 'L', 0,
  /* 2347 */ 'A', 'M', 'O', 'X', 'O', 'R', '_', 'W', '_', 'A', 'Q', '_', 'R', 'L', 0,
  /* 2362 */ 'A', 'M', 'O', 'M', 'I', 'N', 'U', '_', 'W', '_', 'A', 'Q', '_', 'R', 'L', 0,
  /* 2378 */ 'A', 'M', 'O', 'M', 'A', 'X', 'U', '_', 'W', '_', 'A', 'Q', '_', 'R', 'L', 0,
  /* 2394 */ 'A', 'M', 'O', 'M', 'A', 'X', '_', 'W', '_', 'A', 'Q', '_', 'R', 'L', 0,
  /* 2409 */ 'S', 'C', '_', 'W', '_', 'R', 'L', 0,
  /* 2417 */ 'A', 'M', 'O', 'A', 'D', 'D', '_', 'W', '_', 'R', 'L', 0,
  /* 2429 */ 'A', 'M', 'O', 'A', 'N', 'D', '_', 'W', '_', 'R', 'L', 0,
  /* 2441 */ 'A', 'M', 'O', 'M', 'I', 'N', '_', 'W', '_', 'R', 'L', 0,
  /* 2453 */ 'A', 'M', 'O', 'S', 'W', 'A', 'P', '_', 'W', '_', 'R', 'L', 0,
  /* 2466 */ 'L', 'R', '_', 'W', '_', 'R', 'L', 0,
  /* 2474 */ 'A', 'M', 'O', 'O', 'R', '_', 'W', '_', 'R', 'L', 0,
  /* 2485 */ 'A', 'M', 'O', 'X', 'O', 'R', '_', 'W', '_', 'R', 'L', 0,
  /* 2497 */ 'A', 'M', 'O', 'M', 'I', 'N', 'U', '_', 'W', '_', 'R', 'L', 0,
  /* 2510 */ 'A', 'M', 'O', 'M', 'A', 'X', 'U', '_', 'W', '_', 'R', 'L', 0,
  /* 2523 */ 'A', 'M', 'O', 'M', 'A', 'X', '_', 'W', '_', 'R', 'L', 0,
  /* 2535 */ 'G', '_', 'F', 'M', 'U', 'L', 0,
  /* 2542 */ 'G', '_', 'M', 'U', 'L', 0,
  /* 2548 */ 'F', 'C', 'V', 'T', '_', 'D', '_', 'L', 0,
  /* 2557 */ 'F', 'C', 'V', 'T', '_', 'S', '_', 'L', 0,
  /* 2566 */ 'G', '_', 'F', 'R', 'E', 'M', 0,
  /* 2573 */ 'G', '_', 'S', 'R', 'E', 'M', 0,
  /* 2580 */ 'G', '_', 'U', 'R', 'E', 'M', 0,
  /* 2587 */ 'I', 'N', 'L', 'I', 'N', 'E', 'A', 'S', 'M', 0,
  /* 2597 */ 'G', '_', 'F', 'M', 'I', 'N', 'I', 'M', 'U', 'M', 0,
  /* 2608 */ 'G', '_', 'F', 'M', 'A', 'X', 'I', 'M', 'U', 'M', 0,
  /* 2619 */ 'G', '_', 'F', 'M', 'I', 'N', 'N', 'U', 'M', 0,
  /* 2629 */ 'G', '_', 'F', 'M', 'A', 'X', 'N', 'U', 'M', 0,
  /* 2639 */ 'G', '_', 'F', 'C', 'O', 'P', 'Y', 'S', 'I', 'G', 'N', 0,
  /* 2651 */ 'G', '_', 'S', 'M', 'I', 'N', 0,
  /* 2658 */ 'G', '_', 'U', 'M', 'I', 'N', 0,
  /* 2665 */ 'G', '_', 'A', 'T', 'O', 'M', 'I', 'C', 'R', 'M', 'W', '_', 'U', 'M', 'I', 'N', 0,
  /* 2682 */ 'G', '_', 'A', 'T', 'O', 'M', 'I', 'C', 'R', 'M', 'W', '_', 'M', 'I', 'N', 0,
  /* 2698 */ 'G', '_', 'F', 'S', 'I', 'N', 0,
  /* 2705 */ 'C', 'F', 'I', '_', 'I', 'N', 'S', 'T', 'R', 'U', 'C', 'T', 'I', 'O', 'N', 0,
  /* 2721 */ 'C', '_', 'A', 'D', 'D', 'I', '4', 'S', 'P', 'N', 0,
  /* 2732 */ 'A', 'D', 'J', 'C', 'A', 'L', 'L', 'S', 'T', 'A', 'C', 'K', 'D', 'O', 'W', 'N', 0,
  /* 2749 */ 'G', '_', 'S', 'S', 'U', 'B', 'O', 0,
  /* 2757 */ 'G', '_', 'U', 'S', 'U', 'B', 'O', 0,
  /* 2765 */ 'G', '_', 'S', 'A', 'D', 'D', 'O', 0,
  /* 2773 */ 'G', '_', 'U', 'A', 'D', 'D', 'O', 0,
  /* 2781 */ 'G', '_', 'S', 'M', 'U', 'L', 'O', 0,
  /* 2789 */ 'G', '_', 'U', 'M', 'U', 'L', 'O', 0,
  /* 2797 */ 'C', '_', 'A', 'D', 'D', 'I', '_', 'H', 'I', 'N', 'T', '_', 'I', 'M', 'M', '_', 'Z', 'E', 'R', 'O', 0,
  /* 2818 */ 'F', 'E', 'N', 'C', 'E', '_', 'T', 'S', 'O', 0,
  /* 2828 */ 'S', 'T', 'A', 'C', 'K', 'M', 'A', 'P', 0,
  /* 2837 */ 'G', '_', 'B', 'S', 'W', 'A', 'P', 0,
  /* 2845 */ 'G', '_', 'S', 'I', 'T', 'O', 'F', 'P', 0,
  /* 2854 */ 'G', '_', 'U', 'I', 'T', 'O', 'F', 'P', 0,
  /* 2863 */ 'G', '_', 'F', 'C', 'M', 'P', 0,
  /* 2870 */ 'G', '_', 'I', 'C', 'M', 'P', 0,
  /* 2877 */ 'C', '_', 'U', 'N', 'I', 'M', 'P', 0,
  /* 2885 */ 'C', '_', 'N', 'O', 'P', 0,
  /* 2891 */ 'C', '_', 'A', 'D', 'D', 'I', '_', 'N', 'O', 'P', 0,
  /* 2902 */ 'G', '_', 'C', 'T', 'P', 'O', 'P', 0,
  /* 2910 */ 'P', 'A', 'T', 'C', 'H', 'A', 'B', 'L', 'E', '_', 'O', 'P', 0,
  /* 2923 */ 'F', 'A', 'U', 'L', 'T', 'I', 'N', 'G', '_', 'O', 'P', 0,
  /* 2935 */ 'C', '_', 'A', 'D', 'D', 'I', '1', '6', 'S', 'P', 0,
  /* 2946 */ 'C', '_', 'F', 'L', 'D', 'S', 'P', 0,
  /* 2954 */ 'C', '_', 'L', 'D', 'S', 'P', 0,
  /* 2961 */ 'C', '_', 'F', 'S', 'D', 'S', 'P', 0,
  /* 2969 */ 'C', '_', 'S', 'D', 'S', 'P', 0,
  /* 2976 */ 'C', '_', 'F', 'L', 'W', 'S', 'P', 0,
  /* 2984 */ 'C', '_', 'L', 'W', 'S', 'P', 0,
  /* 2991 */ 'C', '_', 'F', 'S', 'W', 'S', 'P', 0,
  /* 2999 */ 'C', '_', 'S', 'W', 'S', 'P', 0,
  /* 3006 */ 'A', 'D', 'J', 'C', 'A', 'L', 'L', 'S', 'T', 'A', 'C', 'K', 'U', 'P', 0,
  /* 3021 */ 'G', '_', 'F', 'E', 'X', 'P', 0,
  /* 3028 */ 'S', 'C', '_', 'D', '_', 'A', 'Q', 0,
  /* 3036 */ 'A', 'M', 'O', 'A', 'D', 'D', '_', 'D', '_', 'A', 'Q', 0,
  /* 3048 */ 'A', 'M', 'O', 'A', 'N', 'D', '_', 'D', '_', 'A', 'Q', 0,
  /* 3060 */ 'A', 'M', 'O', 'M', 'I', 'N', '_', 'D', '_', 'A', 'Q', 0,
  /* 3072 */ 'A', 'M', 'O', 'S', 'W', 'A', 'P', '_', 'D', '_', 'A', 'Q', 0,
  /* 3085 */ 'L', 'R', '_', 'D', '_', 'A', 'Q', 0,
  /* 3093 */ 'A', 'M', 'O', 'O', 'R', '_', 'D', '_', 'A', 'Q', 0,
  /* 3104 */ 'A', 'M', 'O', 'X', 'O', 'R', '_', 'D', '_', 'A', 'Q', 0,
  /* 3116 */ 'A', 'M', 'O', 'M', 'I', 'N', 'U', '_', 'D', '_', 'A', 'Q', 0,
  /* 3129 */ 'A', 'M', 'O', 'M', 'A', 'X', 'U', '_', 'D', '_', 'A', 'Q', 0,
  /* 3142 */ 'A', 'M', 'O', 'M', 'A', 'X', '_', 'D', '_', 'A', 'Q', 0,
  /* 3154 */ 'S', 'C', '_', 'W', '_', 'A', 'Q', 0,
  /* 3162 */ 'A', 'M', 'O', 'A', 'D', 'D', '_', 'W', '_', 'A', 'Q', 0,
  /* 3174 */ 'A', 'M', 'O', 'A', 'N', 'D', '_', 'W', '_', 'A', 'Q', 0,
  /* 3186 */ 'A', 'M', 'O', 'M', 'I', 'N', '_', 'W', '_', 'A', 'Q', 0,
  /* 3198 */ 'A', 'M', 'O', 'S', 'W', 'A', 'P', '_', 'W', '_', 'A', 'Q', 0,
  /* 3211 */ 'L', 'R', '_', 'W', '_', 'A', 'Q', 0,
  /* 3219 */ 'A', 'M', 'O', 'O', 'R', '_', 'W', '_', 'A', 'Q', 0,
  /* 3230 */ 'A', 'M', 'O', 'X', 'O', 'R', '_', 'W', '_', 'A', 'Q', 0,
  /* 3242 */ 'A', 'M', 'O', 'M', 'I', 'N', 'U', '_', 'W', '_', 'A', 'Q', 0,
  /* 3255 */ 'A', 'M', 'O', 'M', 'A', 'X', 'U', '_', 'W', '_', 'A', 'Q', 0,
  /* 3268 */ 'A', 'M', 'O', 'M', 'A', 'X', '_', 'W', '_', 'A', 'Q', 0,
  /* 3280 */ 'B', 'E', 'Q', 0,
  /* 3284 */ 'G', '_', 'B', 'R', 0,
  /* 3289 */ 'I', 'N', 'L', 'I', 'N', 'E', 'A', 'S', 'M', '_', 'B', 'R', 0,
  /* 3302 */ 'P', 's', 'e', 'u', 'd', 'o', 'B', 'R', 0,
  /* 3311 */ 'G', '_', 'B', 'L', 'O', 'C', 'K', '_', 'A', 'D', 'D', 'R', 0,
  /* 3324 */ 'P', 'A', 'T', 'C', 'H', 'A', 'B', 'L', 'E', '_', 'F', 'U', 'N', 'C', 'T', 'I', 'O', 'N', '_', 'E', 'N', 'T', 'E', 'R', 0,
  /* 3349 */ 'G', '_', 'R', 'E', 'A', 'D', 'C', 'Y', 'C', 'L', 'E', 'C', 'O', 'U', 'N', 'T', 'E', 'R', 0,
  /* 3368 */ 'G', '_', 'R', 'E', 'A', 'D', '_', 'R', 'E', 'G', 'I', 'S', 'T', 'E', 'R', 0,
  /* 3384 */ 'G', '_', 'W', 'R', 'I', 'T', 'E', '_', 'R', 'E', 'G', 'I', 'S', 'T', 'E', 'R', 0,
  /* 3401 */ 'G', '_', 'A', 'S', 'H', 'R', 0,
  /* 3408 */ 'G', '_', 'L', 'S', 'H', 'R', 0,
  /* 3415 */ 'C', '_', 'J', 'R', 0,
  /* 3420 */ 'C', '_', 'J', 'A', 'L', 'R', 0,
  /* 3427 */ 'G', '_', 'F', 'F', 'L', 'O', 'O', 'R', 0,
  /* 3436 */ 'G', '_', 'B', 'U', 'I', 'L', 'D', '_', 'V', 'E', 'C', 'T', 'O', 'R', 0,
  /* 3451 */ 'G', '_', 'S', 'H', 'U', 'F', 'F', 'L', 'E', '_', 'V', 'E', 'C', 'T', 'O', 'R', 0,
  /* 3468 */ 'C', '_', 'X', 'O', 'R', 0,
  /* 3474 */ 'G', '_', 'X', 'O', 'R', 0,
  /* 3480 */ 'G', '_', 'A', 'T', 'O', 'M', 'I', 'C', 'R', 'M', 'W', '_', 'X', 'O', 'R', 0,
  /* 3496 */ 'C', '_', 'O', 'R', 0,
  /* 3501 */ 'G', '_', 'O', 'R', 0,
  /* 3506 */ 'G', '_', 'A', 'T', 'O', 'M', 'I', 'C', 'R', 'M', 'W', '_', 'O', 'R', 0,
  /* 3521 */ 'S', 'e', 'l', 'e', 'c', 't', '_', 'F', 'P', 'R', '3', '2', '_', 'U', 's', 'i', 'n', 'g', '_', 'C', 'C', '_', 'G', 'P', 'R', 0,
  /* 3547 */ 'S', 'e', 'l', 'e', 'c', 't', '_', 'F', 'P', 'R', '6', '4', '_', 'U', 's', 'i', 'n', 'g', '_', 'C', 'C', '_', 'G', 'P', 'R', 0,
  /* 3573 */ 'S', 'e', 'l', 'e', 'c', 't', '_', 'G', 'P', 'R', '_', 'U', 's', 'i', 'n', 'g', '_', 'C', 'C', '_', 'G', 'P', 'R', 0,
  /* 3597 */ 'G', '_', 'I', 'N', 'T', 'T', 'O', 'P', 'T', 'R', 0,
  /* 3608 */ 'G', '_', 'F', 'A', 'B', 'S', 0,
  /* 3615 */ 'G', '_', 'U', 'N', 'M', 'E', 'R', 'G', 'E', '_', 'V', 'A', 'L', 'U', 'E', 'S', 0,
  /* 3632 */ 'G', '_', 'M', 'E', 'R', 'G', 'E', '_', 'V', 'A', 'L', 'U', 'E', 'S', 0,
  /* 3647 */ 'G', '_', 'F', 'C', 'O', 'S', 0,
  /* 3654 */ 'G', '_', 'C', 'O', 'N', 'C', 'A', 'T', '_', 'V', 'E', 'C', 'T', 'O', 'R', 'S', 0,
  /* 3671 */ 'C', 'S', 'R', 'R', 'S', 0,
  /* 3677 */ 'C', 'O', 'P', 'Y', '_', 'T', 'O', '_', 'R', 'E', 'G', 'C', 'L', 'A', 'S', 'S', 0,
  /* 3694 */ 'G', '_', 'A', 'T', 'O', 'M', 'I', 'C', '_', 'C', 'M', 'P', 'X', 'C', 'H', 'G', '_', 'W', 'I', 'T', 'H', '_', 'S', 'U', 'C', 'C', 'E', 'S', 'S', 0,
  /* 3724 */ 'G', '_', 'I', 'N', 'T', 'R', 'I', 'N', 'S', 'I', 'C', '_', 'W', '_', 'S', 'I', 'D', 'E', '_', 'E', 'F', 'F', 'E', 'C', 'T', 'S', 0,
  /* 3751 */ 'F', 'S', 'U', 'B', '_', 'S', 0,
  /* 3758 */ 'F', 'M', 'S', 'U', 'B', '_', 'S', 0,
  /* 3766 */ 'F', 'N', 'M', 'S', 'U', 'B', '_', 'S', 0,
  /* 3775 */ 'F', 'A', 'D', 'D', '_', 'S', 0,
  /* 3782 */ 'F', 'M', 'A', 'D', 'D', '_', 'S', 0,
  /* 3790 */ 'F', 'N', 'M', 'A', 'D', 'D', '_', 'S', 0,
  /* 3799 */ 'F', 'C', 'V', 'T', '_', 'D', '_', 'S', 0,
  /* 3808 */ 'F', 'L', 'E', '_', 'S', 0,
  /* 3814 */ 'F', 'S', 'G', 'N', 'J', '_', 'S', 0,
  /* 3822 */ 'F', 'M', 'U', 'L', '_', 'S', 0,
  /* 3829 */ 'F', 'C', 'V', 'T', '_', 'L', '_', 'S', 0,
  /* 3838 */ 'F', 'M', 'I', 'N', '_', 'S', 0,
  /* 3845 */ 'F', 'S', 'G', 'N', 'J', 'N', '_', 'S', 0,
  /* 3854 */ 'F', 'E', 'Q', '_', 'S', 0,
  /* 3860 */ 'F', 'C', 'L', 'A', 'S', 'S', '_', 'S', 0,
  /* 3869 */ 'F', 'L', 'T', '_', 'S', 0,
  /* 3875 */ 'F', 'S', 'Q', 'R', 'T', '_', 'S', 0,
  /* 3883 */ 'F', 'C', 'V', 'T', '_', 'L', 'U', '_', 'S', 0,
  /* 3893 */ 'F', 'C', 'V', 'T', '_', 'W', 'U', '_', 'S', 0,
  /* 3903 */ 'F', 'D', 'I', 'V', '_', 'S', 0,
  /* 3910 */ 'F', 'C', 'V', 'T', '_', 'W', '_', 'S', 0,
  /* 3919 */ 'F', 'M', 'A', 'X', '_', 'S', 0,
  /* 3926 */ 'F', 'S', 'G', 'N', 'J', 'X', '_', 'S', 0,
  /* 3935 */ 'G', '_', 'E', 'X', 'T', 'R', 'A', 'C', 'T', 0,
  /* 3945 */ 'G', '_', 'S', 'E', 'L', 'E', 'C', 'T', 0,
  /* 3954 */ 'G', '_', 'B', 'R', 'I', 'N', 'D', 'I', 'R', 'E', 'C', 'T', 0,
  /* 3967 */ 'M', 'R', 'E', 'T', 0,
  /* 3972 */ 'S', 'R', 'E', 'T', 0,
  /* 3977 */ 'U', 'R', 'E', 'T', 0,
  /* 3982 */ 'P', 'A', 'T', 'C', 'H', 'A', 'B', 'L', 'E', '_', 'R', 'E', 'T', 0,
  /* 3996 */ 'P', 's', 'e', 'u', 'd', 'o', 'R', 'E', 'T', 0,
  /* 4006 */ 'P', 'A', 'T', 'C', 'H', 'A', 'B', 'L', 'E', '_', 'F', 'U', 'N', 'C', 'T', 'I', 'O', 'N', '_', 'E', 'X', 'I', 'T', 0,
  /* 4030 */ 'G', '_', 'B', 'R', 'J', 'T', 0,
  /* 4037 */ 'B', 'L', 'T', 0,
  /* 4041 */ 'G', '_', 'E', 'X', 'T', 'R', 'A', 'C', 'T', '_', 'V', 'E', 'C', 'T', 'O', 'R', '_', 'E', 'L', 'T', 0,
  /* 4062 */ 'G', '_', 'I', 'N', 'S', 'E', 'R', 'T', '_', 'V', 'E', 'C', 'T', 'O', 'R', '_', 'E', 'L', 'T', 0,
  /* 4082 */ 'S', 'L', 'T', 0,
  /* 4086 */ 'G', '_', 'F', 'C', 'O', 'N', 'S', 'T', 'A', 'N', 'T', 0,
  /* 4098 */ 'G', '_', 'C', 'O', 'N', 'S', 'T', 'A', 'N', 'T', 0,
  /* 4109 */ 'C', '_', 'S', 'R', 'A', 'I', '6', '4', '_', 'H', 'I', 'N', 'T', 0,
  /* 4123 */ 'C', '_', 'S', 'L', 'L', 'I', '6', '4', '_', 'H', 'I', 'N', 'T', 0,
  /* 4137 */ 'C', '_', 'S', 'R', 'L', 'I', '6', '4', '_', 'H', 'I', 'N', 'T', 0,
  /* 4151 */ 'C', '_', 'A', 'D', 'D', '_', 'H', 'I', 'N', 'T', 0,
  /* 4162 */ 'C', '_', 'S', 'L', 'L', 'I', '_', 'H', 'I', 'N', 'T', 0,
  /* 4174 */ 'C', '_', 'L', 'I', '_', 'H', 'I', 'N', 'T', 0,
  /* 4184 */ 'C', '_', 'L', 'U', 'I', '_', 'H', 'I', 'N', 'T', 0,
  /* 4195 */ 'C', '_', 'N', 'O', 'P', '_', 'H', 'I', 'N', 'T', 0,
  /* 4206 */ 'C', '_', 'M', 'V', '_', 'H', 'I', 'N', 'T', 0,
  /* 4216 */ 'S', 'T', 'A', 'T', 'E', 'P', 'O', 'I', 'N', 'T', 0,
  /* 4227 */ 'P', 'A', 'T', 'C', 'H', 'P', 'O', 'I', 'N', 'T', 0,
  /* 4238 */ 'G', '_', 'P', 'T', 'R', 'T', 'O', 'I', 'N', 'T', 0,
  /* 4249 */ 'G', '_', 'F', 'R', 'I', 'N', 'T', 0,
  /* 4257 */ 'G', '_', 'F', 'N', 'E', 'A', 'R', 'B', 'Y', 'I', 'N', 'T', 0,
  /* 4270 */ 'G', '_', 'V', 'A', 'S', 'T', 'A', 'R', 'T', 0,
  /* 4280 */ 'L', 'I', 'F', 'E', 'T', 'I', 'M', 'E', '_', 'S', 'T', 'A', 'R', 'T', 0,
  /* 4295 */ 'G', '_', 'I', 'N', 'S', 'E', 'R', 'T', 0,
  /* 4304 */ 'G', '_', 'F', 'S', 'Q', 'R', 'T', 0,
  /* 4312 */ 'G', '_', 'B', 'I', 'T', 'C', 'A', 'S', 'T', 0,
  /* 4322 */ 'G', '_', 'A', 'D', 'D', 'R', 'S', 'P', 'A', 'C', 'E', '_', 'C', 'A', 'S', 'T', 0,
  /* 4339 */ 'G', '_', 'F', 'P', 'E', 'X', 'T', 0,
  /* 4347 */ 'G', '_', 'S', 'E', 'X', 'T', 0,
  /* 4354 */ 'G', '_', 'A', 'N', 'Y', 'E', 'X', 'T', 0,
  /* 4363 */ 'G', '_', 'Z', 'E', 'X', 'T', 0,
  /* 4370 */ 'P', 's', 'e', 'u', 'd', 'o', 'L', 'B', 'U', 0,
  /* 4380 */ 'B', 'G', 'E', 'U', 0,
  /* 4385 */ 'M', 'U', 'L', 'H', 'U', 0,
  /* 4391 */ 'P', 's', 'e', 'u', 'd', 'o', 'L', 'H', 'U', 0,
  /* 4401 */ 'S', 'L', 'T', 'I', 'U', 0,
  /* 4407 */ 'F', 'C', 'V', 'T', '_', 'D', '_', 'L', 'U', 0,
  /* 4417 */ 'F', 'C', 'V', 'T', '_', 'S', '_', 'L', 'U', 0,
  /* 4427 */ 'R', 'E', 'M', 'U', 0,
  /* 4432 */ 'M', 'U', 'L', 'H', 'S', 'U', 0,
  /* 4439 */ 'B', 'L', 'T', 'U', 0,
  /* 4444 */ 'S', 'L', 'T', 'U', 0,
  /* 4449 */ 'D', 'I', 'V', 'U', 0,
  /* 4454 */ 'P', 's', 'e', 'u', 'd', 'o', 'L', 'W', 'U', 0,
  /* 4464 */ 'F', 'C', 'V', 'T', '_', 'D', '_', 'W', 'U', 0,
  /* 4474 */ 'F', 'C', 'V', 'T', '_', 'S', '_', 'W', 'U', 0,
  /* 4484 */ 'G', '_', 'F', 'D', 'I', 'V', 0,
  /* 4491 */ 'G', '_', 'S', 'D', 'I', 'V', 0,
  /* 4498 */ 'G', '_', 'U', 'D', 'I', 'V', 0,
  /* 4505 */ 'C', '_', 'M', 'V', 0,
  /* 4510 */ 'S', 'R', 'A', 'W', 0,
  /* 4515 */ 'C', '_', 'S', 'U', 'B', 'W', 0,
  /* 4522 */ 'C', '_', 'A', 'D', 'D', 'W', 0,
  /* 4529 */ 'S', 'R', 'A', 'I', 'W', 0,
  /* 4535 */ 'C', '_', 'A', 'D', 'D', 'I', 'W', 0,
  /* 4543 */ 'S', 'L', 'L', 'I', 'W', 0,
  /* 4549 */ 'S', 'R', 'L', 'I', 'W', 0,
  /* 4555 */ 'C', '_', 'F', 'L', 'W', 0,
  /* 4561 */ 'P', 's', 'e', 'u', 'd', 'o', 'F', 'L', 'W', 0,
  /* 4571 */ 'S', 'L', 'L', 'W', 0,
  /* 4576 */ 'S', 'R', 'L', 'W', 0,
  /* 4581 */ 'M', 'U', 'L', 'W', 0,
  /* 4586 */ 'C', '_', 'L', 'W', 0,
  /* 4591 */ 'P', 's', 'e', 'u', 'd', 'o', 'L', 'W', 0,
  /* 4600 */ 'R', 'E', 'M', 'W', 0,
  /* 4605 */ 'G', '_', 'F', 'P', 'O', 'W', 0,
  /* 4612 */ 'C', 'S', 'R', 'R', 'W', 0,
  /* 4618 */ 'C', '_', 'F', 'S', 'W', 0,
  /* 4624 */ 'P', 's', 'e', 'u', 'd', 'o', 'F', 'S', 'W', 0,
  /* 4634 */ 'C', '_', 'S', 'W', 0,
  /* 4639 */ 'P', 's', 'e', 'u', 'd', 'o', 'S', 'W', 0,
  /* 4648 */ 'R', 'E', 'M', 'U', 'W', 0,
  /* 4654 */ 'D', 'I', 'V', 'U', 'W', 0,
  /* 4660 */ 'D', 'I', 'V', 'W', 0,
  /* 4665 */ 'S', 'C', '_', 'W', 0,
  /* 4670 */ 'A', 'M', 'O', 'A', 'D', 'D', '_', 'W', 0,
  /* 4679 */ 'A', 'M', 'O', 'A', 'N', 'D', '_', 'W', 0,
  /* 4688 */ 'F', 'C', 'V', 'T', '_', 'D', '_', 'W', 0,
  /* 4697 */ 'A', 'M', 'O', 'M', 'I', 'N', '_', 'W', 0,
  /* 4706 */ 'A', 'M', 'O', 'S', 'W', 'A', 'P', '_', 'W', 0,
  /* 4716 */ 'L', 'R', '_', 'W', 0,
  /* 4721 */ 'A', 'M', 'O', 'O', 'R', '_', 'W', 0,
  /* 4729 */ 'A', 'M', 'O', 'X', 'O', 'R', '_', 'W', 0,
  /* 4738 */ 'F', 'C', 'V', 'T', '_', 'S', '_', 'W', 0,
  /* 4747 */ 'A', 'M', 'O', 'M', 'I', 'N', 'U', '_', 'W', 0,
  /* 4757 */ 'A', 'M', 'O', 'M', 'A', 'X', 'U', '_', 'W', 0,
  /* 4767 */ 'A', 'M', 'O', 'M', 'A', 'X', '_', 'W', 0,
  /* 4776 */ 'F', 'M', 'V', '_', 'X', '_', 'W', 0,
  /* 4784 */ 'G', '_', 'S', 'M', 'A', 'X', 0,
  /* 4791 */ 'G', '_', 'U', 'M', 'A', 'X', 0,
  /* 4798 */ 'G', '_', 'A', 'T', 'O', 'M', 'I', 'C', 'R', 'M', 'W', '_', 'U', 'M', 'A', 'X', 0,
  /* 4815 */ 'G', '_', 'A', 'T', 'O', 'M', 'I', 'C', 'R', 'M', 'W', '_', 'M', 'A', 'X', 0,
  /* 4831 */ 'G', '_', 'F', 'R', 'A', 'M', 'E', '_', 'I', 'N', 'D', 'E', 'X', 0,
  /* 4845 */ 'F', 'M', 'V', '_', 'D', '_', 'X', 0,
  /* 4853 */ 'F', 'M', 'V', '_', 'W', '_', 'X', 0,
  /* 4861 */ 'C', 'O', 'P', 'Y', 0,
  /* 4866 */ 'C', '_', 'B', 'N', 'E', 'Z', 0,
  /* 4873 */ 'G', '_', 'C', 'T', 'L', 'Z', 0,
  /* 4880 */ 'C', '_', 'B', 'E', 'Q', 'Z', 0,
  /* 4887 */ 'G', '_', 'C', 'T', 'T', 'Z', 0,
  /* 4894 */ 'R', 'e', 'a', 'd', 'C', 'y', 'c', 'l', 'e', 'W', 'i', 'd', 'e', 0,
  /* 4908 */ 'P', 's', 'e', 'u', 'd', 'o', 'C', 'A', 'L', 'L', 'R', 'e', 'g', 0,
  /* 4922 */ 'P', 's', 'e', 'u', 'd', 'o', 'A', 'd', 'd', 'T', 'P', 'R', 'e', 'l', 0,
  /* 4937 */ 'B', 'u', 'i', 'l', 'd', 'P', 'a', 'i', 'r', 'F', '6', '4', 'P', 's', 'e', 'u', 'd', 'o', 0,
  /* 4956 */ 'S', 'p', 'l', 'i', 't', 'F', '6', '4', 'P', 's', 'e', 'u', 'd', 'o', 0,
  /* 4971 */ 'P', 's', 'e', 'u', 'd', 'o', 'T', 'A', 'I', 'L', 'I', 'n', 'd', 'i', 'r', 'e', 'c', 't', 0,
  /* 4990 */ 'P', 's', 'e', 'u', 'd', 'o', 'C', 'A', 'L', 'L', 'I', 'n', 'd', 'i', 'r', 'e', 'c', 't', 0,
};

extern const unsigned RISCVInstrNameIndices[] = {
    1647U, 2587U, 3289U, 2705U, 1784U, 1765U, 1793U, 1952U,
    1474U, 1489U, 1454U, 1516U, 3677U, 1375U, 1774U, 1222U,
    4861U, 1314U, 4280U, 814U, 2828U, 1929U, 4227U, 866U,
    4216U, 1325U, 2923U, 2910U, 3324U, 3982U, 4006U, 1861U,
    1908U, 1881U, 1810U, 691U, 452U, 2542U, 4491U, 4498U,
    2573U, 2580U, 792U, 3501U, 3474U, 1452U, 1645U, 4831U,
    1385U, 3935U, 3615U, 4295U, 3632U, 3436U, 522U, 3654U,
    4238U, 3597U, 4312U, 496U, 848U, 3349U, 654U, 598U,
    628U, 639U, 579U, 609U, 1354U, 1338U, 3694U, 1530U,
    1547U, 707U, 458U, 798U, 769U, 3506U, 3480U, 4815U,
    2682U, 4798U, 2665U, 668U, 429U, 1214U, 839U, 3954U,
    474U, 3724U, 4354U, 514U, 4098U, 4086U, 4270U, 1571U,
    4347U, 1503U, 4363U, 1830U, 3408U, 3401U, 2870U, 2863U,
    3945U, 2773U, 1243U, 2757U, 1206U, 2765U, 1235U, 2749U,
    1198U, 2789U, 2781U, 1587U, 1579U, 661U, 422U, 2535U,
    383U, 572U, 4484U, 2566U, 4605U, 3021U, 317U, 1564U,
    309U, 0U, 1467U, 4339U, 486U, 1684U, 1711U, 2845U,
    2854U, 3608U, 2639U, 1400U, 2619U, 2629U, 1251U, 1266U,
    2597U, 2608U, 697U, 1748U, 2651U, 4784U, 2658U, 4791U,
    3284U, 4030U, 4062U, 4041U, 3451U, 4887U, 1434U, 4873U,
    1416U, 2902U, 2837U, 1362U, 1847U, 3647U, 2698U, 4304U,
    3427U, 4249U, 4257U, 4322U, 3311U, 1301U, 543U, 3368U,
    3384U, 2732U, 3006U, 4937U, 4922U, 109U, 325U, 3302U,
    827U, 1941U, 4990U, 4908U, 154U, 348U, 745U, 4561U,
    889U, 4624U, 374U, 723U, 1285U, 404U, 4370U, 760U,
    1595U, 4391U, 1670U, 364U, 4591U, 4454U, 52U, 281U,
    199U, 80U, 24U, 252U, 170U, 227U, 132U, 3996U,
    413U, 904U, 1604U, 4639U, 1836U, 4971U, 4894U, 3521U,
    3547U, 3573U, 4956U, 664U, 1629U, 4537U, 4524U, 966U,
    3036U, 2102U, 1973U, 4670U, 3162U, 2261U, 2417U, 975U,
    3048U, 2117U, 1985U, 4679U, 3174U, 2276U, 2429U, 1139U,
    3129U, 2219U, 2066U, 4757U, 3255U, 2378U, 2510U, 1172U,
    3142U, 2235U, 2079U, 4767U, 3268U, 2394U, 2523U, 1119U,
    3116U, 2203U, 2053U, 4747U, 3242U, 2362U, 2497U, 1021U,
    3060U, 2132U, 1997U, 4697U, 3186U, 2291U, 2441U, 1060U,
    3093U, 2174U, 2030U, 4721U, 3219U, 2333U, 2474U, 1039U,
    3072U, 2147U, 2009U, 4706U, 3198U, 2306U, 2453U, 1068U,
    3104U, 2188U, 2041U, 4729U, 3230U, 2347U, 2485U, 782U,
    1636U, 560U, 3280U, 1281U, 4380U, 4037U, 4439U, 1321U,
    566U, 1620U, 3671U, 1693U, 4612U, 1720U, 685U, 1627U,
    2935U, 2721U, 4535U, 2797U, 9U, 2891U, 4522U, 4151U,
    786U, 1634U, 4880U, 4866U, 1739U, 739U, 2946U, 4555U,
    2976U, 883U, 2961U, 4618U, 2991U, 1735U, 1759U, 3420U,
    3415U, 755U, 2954U, 1665U, 4174U, 1705U, 4184U, 4586U,
    2984U, 4505U, 4206U, 2885U, 4195U, 3496U, 899U, 2969U,
    1651U, 4123U, 4162U, 1613U, 4109U, 1658U, 4137U, 446U,
    4515U, 4634U, 2999U, 2877U, 3468U, 4487U, 4449U, 4654U,
    4660U, 1741U, 1855U, 942U, 3775U, 1077U, 3860U, 2548U,
    4407U, 3799U, 4688U, 4464U, 1109U, 3883U, 1005U, 3829U,
    1086U, 2557U, 4417U, 4738U, 4474U, 1129U, 3893U, 1156U,
    3910U, 1149U, 3903U, 1216U, 1727U, 2818U, 1049U, 3854U,
    741U, 984U, 3808U, 1095U, 3869U, 4557U, 949U, 3782U,
    1165U, 3919U, 1014U, 3838U, 920U, 3758U, 998U, 3822U,
    4845U, 4853U, 1190U, 4776U, 957U, 3790U, 928U, 3766U,
    885U, 1030U, 3845U, 1181U, 3926U, 990U, 3814U, 1101U,
    3875U, 913U, 3751U, 4620U, 1761U, 3422U, 410U, 4376U,
    742U, 1584U, 4387U, 1055U, 3085U, 2163U, 2022U, 4716U,
    3211U, 2322U, 2466U, 1707U, 4558U, 4460U, 3967U, 2538U,
    1582U, 4432U, 4385U, 4581U, 3433U, 1680U, 2569U, 4427U,
    4648U, 4600U, 419U, 937U, 3028U, 2091U, 1965U, 4665U,
    3154U, 2250U, 2409U, 886U, 389U, 1610U, 1957U, 1653U,
    4543U, 4571U, 4082U, 1700U, 4401U, 4444U, 400U, 1615U,
    4529U, 4510U, 3972U, 1961U, 1660U, 4549U, 4576U, 425U,
    4517U, 4621U, 2879U, 3977U, 1641U, 3470U, 1679U,
};

static inline void InitRISCVMCInstrInfo(MCInstrInfo *II) {
  II->InitMCInstrInfo(RISCVInsts, RISCVInstrNameIndices, RISCVInstrNameData, 511);
}

} // end namespace llvm
#endif // GET_INSTRINFO_MC_DESC

#ifdef GET_INSTRINFO_HEADER
#undef GET_INSTRINFO_HEADER
namespace llvm {
struct RISCVGenInstrInfo : public TargetInstrInfo {
  explicit RISCVGenInstrInfo(int CFSetupOpcode = -1, int CFDestroyOpcode = -1, int CatchRetOpcode = -1, int ReturnOpcode = -1);
  ~RISCVGenInstrInfo() override = default;

};
} // end namespace llvm
#endif // GET_INSTRINFO_HEADER

#ifdef GET_INSTRINFO_HELPER_DECLS
#undef GET_INSTRINFO_HELPER_DECLS


#endif // GET_INSTRINFO_HELPER_DECLS

#ifdef GET_INSTRINFO_HELPERS
#undef GET_INSTRINFO_HELPERS

#endif // GET_INSTRINFO_HELPERS

#ifdef GET_INSTRINFO_CTOR_DTOR
#undef GET_INSTRINFO_CTOR_DTOR
namespace llvm {
extern const MCInstrDesc RISCVInsts[];
extern const unsigned RISCVInstrNameIndices[];
extern const char RISCVInstrNameData[];
RISCVGenInstrInfo::RISCVGenInstrInfo(int CFSetupOpcode, int CFDestroyOpcode, int CatchRetOpcode, int ReturnOpcode)
  : TargetInstrInfo(CFSetupOpcode, CFDestroyOpcode, CatchRetOpcode, ReturnOpcode) {
  InitMCInstrInfo(RISCVInsts, RISCVInstrNameIndices, RISCVInstrNameData, 511);
}
} // end namespace llvm
#endif // GET_INSTRINFO_CTOR_DTOR

#ifdef GET_INSTRINFO_OPERAND_ENUM
#undef GET_INSTRINFO_OPERAND_ENUM
namespace llvm {
namespace RISCV {
namespace OpName {
enum {
OPERAND_LAST
};
} // end namespace OpName
} // end namespace RISCV
} // end namespace llvm
#endif //GET_INSTRINFO_OPERAND_ENUM

#ifdef GET_INSTRINFO_NAMED_OPS
#undef GET_INSTRINFO_NAMED_OPS
namespace llvm {
namespace RISCV {
LLVM_READONLY
int16_t getNamedOperandIdx(uint16_t Opcode, uint16_t NamedIdx) {
  return -1;
}
} // end namespace RISCV
} // end namespace llvm
#endif //GET_INSTRINFO_NAMED_OPS

#ifdef GET_INSTRINFO_OPERAND_TYPES_ENUM
#undef GET_INSTRINFO_OPERAND_TYPES_ENUM
namespace llvm {
namespace RISCV {
namespace OpTypes {
enum OperandType {
  bare_symbol = 0,
  c_lui_imm = 1,
  call_symbol = 2,
  csr_sysreg = 3,
  f32imm = 4,
  f64imm = 5,
  fencearg = 6,
  frmarg = 7,
  i16imm = 8,
  i1imm = 9,
  i32imm = 10,
  i64imm = 11,
  i8imm = 12,
  immzero = 13,
  ixlenimm = 14,
  ixlenimm_li = 15,
  ptype0 = 16,
  ptype1 = 17,
  ptype2 = 18,
  ptype3 = 19,
  ptype4 = 20,
  ptype5 = 21,
  simm10_lsb0000nonzero = 22,
  simm12 = 23,
  simm12_lsb0 = 24,
  simm13_lsb0 = 25,
  simm21_lsb0_jal = 26,
  simm6 = 27,
  simm6nonzero = 28,
  simm9_lsb0 = 29,
  tprel_add_symbol = 30,
  type0 = 31,
  type1 = 32,
  type2 = 33,
  type3 = 34,
  type4 = 35,
  type5 = 36,
  uimm10_lsb00nonzero = 37,
  uimm20_auipc = 38,
  uimm20_lui = 39,
  uimm5 = 40,
  uimm7_lsb00 = 41,
  uimm8_lsb00 = 42,
  uimm8_lsb000 = 43,
  uimm9_lsb000 = 44,
  uimmlog2xlen = 45,
  uimmlog2xlennonzero = 46,
  untyped_imm_0 = 47,
  GPRMemAtomic = 48,
  FPR32 = 49,
  FPR32C = 50,
  FPR64 = 51,
  FPR64C = 52,
  GPR = 53,
  GPRC = 54,
  GPRNoX0 = 55,
  GPRNoX0X2 = 56,
  GPRTC = 57,
  GPRX0 = 58,
  SP = 59,
  OPERAND_TYPE_LIST_END
};
} // end namespace OpTypes
} // end namespace RISCV
} // end namespace llvm
#endif // GET_INSTRINFO_OPERAND_TYPES_ENUM

#ifdef GET_INSTRINFO_OPERAND_TYPE
#undef GET_INSTRINFO_OPERAND_TYPE
namespace llvm {
namespace RISCV {
LLVM_READONLY
static int getOperandType(uint16_t Opcode, uint16_t OpIdx) {
  const int Offsets[] = {
    0,
    1,
    1,
    1,
    2,
    3,
    4,
    5,
    5,
    8,
    12,
    13,
    17,
    20,
    20,
    21,
    23,
    25,
    25,
    26,
    27,
    29,
    29,
    35,
    36,
    36,
    38,
    39,
    39,
    39,
    39,
    39,
    39,
    41,
    44,
    44,
    47,
    50,
    53,
    56,
    59,
    62,
    65,
    68,
    71,
    74,
    75,
    76,
    78,
    80,
    83,
    85,
    89,
    91,
    93,
    95,
    97,
    99,
    101,
    103,
    105,
    107,
    108,
    110,
    112,
    114,
    119,
    124,
    129,
    131,
    136,
    141,
    145,
    148,
    151,
    154,
    157,
    160,
    163,
    166,
    169,
    172,
    175,
    178,
    181,
    184,
    186,
    188,
    189,
    190,
    191,
    193,
    195,
    197,
    199,
    200,
    203,
    205,
    208,
    210,
    213,
    216,
    219,
    223,
    227,
    231,
    235,
    240,
    244,
    249,
    253,
    258,
    262,
    267,
    271,
    275,
    278,
    281,
    284,
    287,
    290,
    294,
    298,
    301,
    304,
    307,
    309,
    311,
    313,
    315,
    317,
    319,
    321,
    323,
    325,
    327,
    329,
    331,
    333,
    336,
    338,
    341,
    344,
    347,
    350,
    353,
    356,
    359,
    362,
    365,
    368,
    371,
    374,
    375,
    378,
    382,
    385,
    389,
    391,
    393,
    395,
    397,
    399,
    401,
    403,
    405,
    407,
    409,
    411,
    413,
    415,
    417,
    419,
    421,
    423,
    426,
    428,
    430,
    432,
    434,
    437,
    441,
    446,
    451,
    452,
    454,
    455,
    456,
    458,
    464,
    470,
    473,
    476,
    479,
    482,
    484,
    486,
    488,
    490,
    492,
    494,
    496,
    498,
    500,
    502,
    504,
    506,
    512,
    520,
    528,
    534,
    540,
    547,
    554,
    560,
    567,
    567,
    570,
    573,
    576,
    579,
    580,
    581,
    583,
    589,
    595,
    601,
    604,
    607,
    610,
    613,
    616,
    619,
    622,
    625,
    628,
    631,
    634,
    637,
    640,
    643,
    646,
    649,
    652,
    655,
    658,
    661,
    664,
    667,
    670,
    673,
    676,
    679,
    682,
    685,
    688,
    691,
    694,
    697,
    700,
    703,
    706,
    709,
    712,
    715,
    718,
    721,
    724,
    727,
    730,
    733,
    736,
    739,
    742,
    745,
    748,
    751,
    754,
    757,
    760,
    763,
    766,
    769,
    772,
    775,
    778,
    781,
    784,
    787,
    790,
    793,
    796,
    799,
    802,
    805,
    808,
    811,
    814,
    817,
    820,
    823,
    826,
    829,
    832,
    835,
    838,
    840,
    843,
    846,
    849,
    852,
    855,
    858,
    861,
    864,
    867,
    870,
    873,
    876,
    879,
    882,
    885,
    888,
    891,
    894,
    897,
    900,
    903,
    906,
    909,
    912,
    914,
    916,
    916,
    919,
    922,
    925,
    928,
    931,
    934,
    937,
    940,
    941,
    942,
    943,
    944,
    947,
    950,
    952,
    954,
    956,
    958,
    961,
    964,
    966,
    968,
    968,
    969,
    972,
    975,
    978,
    981,
    983,
    986,
    989,
    991,
    994,
    996,
    999,
    1002,
    1005,
    1008,
    1008,
    1011,
    1014,
    1017,
    1020,
    1023,
    1023,
    1023,
    1027,
    1031,
    1033,
    1035,
    1038,
    1041,
    1043,
    1045,
    1047,
    1050,
    1053,
    1056,
    1059,
    1062,
    1065,
    1068,
    1071,
    1074,
    1077,
    1080,
    1083,
    1086,
    1090,
    1094,
    1096,
    1096,
    1096,
    1099,
    1102,
    1105,
    1108,
    1111,
    1114,
    1117,
    1120,
    1125,
    1130,
    1133,
    1136,
    1139,
    1142,
    1147,
    1152,
    1156,
    1160,
    1162,
    1164,
    1166,
    1168,
    1173,
    1178,
    1183,
    1188,
    1191,
    1194,
    1197,
    1200,
    1203,
    1206,
    1209,
    1212,
    1215,
    1219,
    1223,
    1226,
    1228,
    1231,
    1234,
    1237,
    1240,
    1243,
    1246,
    1248,
    1250,
    1252,
    1254,
    1256,
    1258,
    1260,
    1262,
    1264,
    1267,
    1270,
    1272,
    1275,
    1278,
    1281,
    1284,
    1287,
    1290,
    1293,
    1296,
    1299,
    1302,
    1305,
    1308,
    1311,
    1314,
    1317,
    1320,
    1323,
    1326,
    1329,
    1332,
    1335,
    1337,
    1340,
    1343,
    1346,
    1349,
    1352,
    1355,
    1358,
    1361,
    1364,
    1367,
    1370,
    1373,
    1376,
    1378,
    1381,
    1384,
    1387,
    1390,
    1393,
    1396,
    1399,
    1399,
    1401,
    1403,
    1406,
  };
  const int OpcodeOperandTypes[] = {
    -1,
    /**/
    /**/
    OpTypes::i32imm,
    OpTypes::i32imm,
    OpTypes::i32imm,
    OpTypes::i32imm,
    /**/
    -1, -1, OpTypes::i32imm,
    -1, -1, -1, OpTypes::i32imm,
    -1,
    -1, -1, -1, OpTypes::i32imm,
    -1, -1, OpTypes::i32imm,
    /**/
    -1,
    -1, -1,
    -1, -1,
    /**/
    OpTypes::i32imm,
    OpTypes::i32imm,
    OpTypes::i64imm, OpTypes::i32imm,
    /**/
    -1, OpTypes::i64imm, OpTypes::i32imm, -1, OpTypes::i32imm, OpTypes::i32imm,
    -1,
    /**/
    -1, OpTypes::i32imm,
    -1,
    /**/
    /**/
    /**/
    /**/
    /**/
    -1, -1,
    -1, -1, -1,
    /**/
    OpTypes::type0, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0, OpTypes::type0,
    OpTypes::type0,
    OpTypes::type0,
    OpTypes::type0, -1,
    OpTypes::type0, -1,
    OpTypes::type0, OpTypes::type1, -1,
    OpTypes::type0, OpTypes::type1,
    OpTypes::type0, OpTypes::type0, OpTypes::type1, -1,
    OpTypes::type0, OpTypes::type1,
    OpTypes::type0, OpTypes::type1,
    OpTypes::type0, OpTypes::type1,
    OpTypes::type0, OpTypes::type1,
    OpTypes::type0, OpTypes::type1,
    OpTypes::type0, OpTypes::type1,
    OpTypes::type0, OpTypes::type1,
    OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0,
    OpTypes::type0,
    OpTypes::type0, OpTypes::ptype1,
    OpTypes::type0, OpTypes::ptype1,
    OpTypes::type0, OpTypes::ptype1,
    OpTypes::type0, OpTypes::ptype1, OpTypes::ptype1, OpTypes::type2, -1,
    OpTypes::type0, OpTypes::ptype1, OpTypes::ptype1, OpTypes::type2, -1,
    OpTypes::type0, OpTypes::ptype1, OpTypes::ptype1, OpTypes::type2, -1,
    OpTypes::type0, OpTypes::ptype1,
    OpTypes::ptype0, OpTypes::type1, OpTypes::ptype0, OpTypes::ptype2, -1,
    OpTypes::type0, OpTypes::type1, OpTypes::type2, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::ptype1, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::ptype1, OpTypes::type0,
    OpTypes::type0, OpTypes::ptype1, OpTypes::type0,
    OpTypes::type0, OpTypes::ptype1, OpTypes::type0,
    OpTypes::type0, OpTypes::ptype1, OpTypes::type0,
    OpTypes::type0, OpTypes::ptype1, OpTypes::type0,
    OpTypes::type0, OpTypes::ptype1, OpTypes::type0,
    OpTypes::type0, OpTypes::ptype1, OpTypes::type0,
    OpTypes::type0, OpTypes::ptype1, OpTypes::type0,
    OpTypes::type0, OpTypes::ptype1, OpTypes::type0,
    OpTypes::type0, OpTypes::ptype1, OpTypes::type0,
    OpTypes::type0, OpTypes::ptype1, OpTypes::type0,
    OpTypes::type0, OpTypes::ptype1, OpTypes::type0,
    OpTypes::type0, OpTypes::ptype1, OpTypes::type0,
    OpTypes::i32imm, OpTypes::i32imm,
    OpTypes::type0, -1,
    OpTypes::type0,
    -1,
    -1,
    OpTypes::type0, OpTypes::type1,
    OpTypes::type0, OpTypes::type1,
    OpTypes::type0, -1,
    OpTypes::type0, -1,
    OpTypes::type0,
    OpTypes::type0, OpTypes::type1, -1,
    OpTypes::type0, OpTypes::type1,
    OpTypes::type0, OpTypes::type0, OpTypes::untyped_imm_0,
    OpTypes::type0, OpTypes::type1,
    OpTypes::type0, OpTypes::type0, OpTypes::type1,
    OpTypes::type0, OpTypes::type0, OpTypes::type1,
    OpTypes::type0, OpTypes::type0, OpTypes::type1,
    OpTypes::type0, -1, OpTypes::type1, OpTypes::type1,
    OpTypes::type0, -1, OpTypes::type1, OpTypes::type1,
    OpTypes::type0, OpTypes::type1, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type1, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type1, OpTypes::type0, OpTypes::type0, OpTypes::type1,
    OpTypes::type0, OpTypes::type1, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type1, OpTypes::type0, OpTypes::type0, OpTypes::type1,
    OpTypes::type0, OpTypes::type1, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type1, OpTypes::type0, OpTypes::type0, OpTypes::type1,
    OpTypes::type0, OpTypes::type1, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type1, OpTypes::type0, OpTypes::type0, OpTypes::type1,
    OpTypes::type0, OpTypes::type1, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type1, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type1,
    OpTypes::type0, OpTypes::type1,
    OpTypes::type0, OpTypes::type1,
    OpTypes::type0, OpTypes::type1,
    OpTypes::type0, OpTypes::type1,
    OpTypes::type0, OpTypes::type1,
    OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0, OpTypes::type1,
    OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0, OpTypes::type1,
    OpTypes::type0, OpTypes::type0, -1,
    OpTypes::type0, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0, OpTypes::type0,
    -1,
    OpTypes::ptype0, -1, OpTypes::type1,
    OpTypes::type0, OpTypes::type0, OpTypes::type1, OpTypes::type2,
    OpTypes::type0, OpTypes::type1, OpTypes::type2,
    OpTypes::type0, OpTypes::type1, OpTypes::type1, -1,
    OpTypes::type0, OpTypes::type1,
    OpTypes::type0, OpTypes::type1,
    OpTypes::type0, OpTypes::type1,
    OpTypes::type0, OpTypes::type1,
    OpTypes::type0, OpTypes::type1,
    OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type0,
    OpTypes::type0, OpTypes::type1,
    OpTypes::type0, -1,
    OpTypes::type0, -1,
    OpTypes::ptype0, OpTypes::type1, OpTypes::i32imm,
    OpTypes::type0, -1,
    -1, OpTypes::type0,
    OpTypes::i32imm, OpTypes::i32imm,
    OpTypes::i32imm, OpTypes::i32imm,
    OpTypes::FPR64, OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::tprel_add_symbol,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::ixlenimm,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::ixlenimm,
    OpTypes::simm21_lsb0_jal,
    OpTypes::GPR, OpTypes::simm12,
    OpTypes::call_symbol,
    OpTypes::GPR,
    OpTypes::GPR, OpTypes::call_symbol,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::ixlenimm,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::ixlenimm,
    OpTypes::FPR64, OpTypes::GPR, OpTypes::bare_symbol,
    OpTypes::FPR32, OpTypes::GPR, OpTypes::bare_symbol,
    OpTypes::FPR64, OpTypes::GPR, OpTypes::bare_symbol,
    OpTypes::FPR32, OpTypes::GPR, OpTypes::bare_symbol,
    OpTypes::GPR, OpTypes::bare_symbol,
    OpTypes::GPR, OpTypes::bare_symbol,
    OpTypes::GPR, OpTypes::bare_symbol,
    OpTypes::GPR, OpTypes::bare_symbol,
    OpTypes::GPR, OpTypes::bare_symbol,
    OpTypes::GPR, OpTypes::bare_symbol,
    OpTypes::GPR, OpTypes::bare_symbol,
    OpTypes::GPR, OpTypes::bare_symbol,
    OpTypes::GPR, OpTypes::ixlenimm_li,
    OpTypes::GPR, OpTypes::bare_symbol,
    OpTypes::GPR, OpTypes::bare_symbol,
    OpTypes::GPR, OpTypes::bare_symbol,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::ixlenimm,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::ixlenimm, OpTypes::ixlenimm,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::ixlenimm, OpTypes::ixlenimm,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::ixlenimm,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::ixlenimm,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::ixlenimm,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::ixlenimm,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::ixlenimm,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::ixlenimm,
    /**/
    OpTypes::GPR, OpTypes::GPR, OpTypes::bare_symbol,
    OpTypes::GPR, OpTypes::GPR, OpTypes::bare_symbol,
    OpTypes::GPR, OpTypes::GPR, OpTypes::bare_symbol,
    OpTypes::GPR, OpTypes::GPR, OpTypes::bare_symbol,
    OpTypes::call_symbol,
    OpTypes::GPRTC,
    OpTypes::GPR, OpTypes::GPR,
    OpTypes::FPR32, OpTypes::GPR, OpTypes::GPR, OpTypes::ixlenimm, OpTypes::FPR32, OpTypes::FPR32,
    OpTypes::FPR64, OpTypes::GPR, OpTypes::GPR, OpTypes::ixlenimm, OpTypes::FPR64, OpTypes::FPR64,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR, OpTypes::ixlenimm, OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::FPR64,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::simm12,
    OpTypes::GPR, OpTypes::GPR, OpTypes::simm12,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::simm12,
    OpTypes::GPR, OpTypes::uimm20_auipc,
    OpTypes::GPR, OpTypes::GPR, OpTypes::simm13_lsb0,
    OpTypes::GPR, OpTypes::GPR, OpTypes::simm13_lsb0,
    OpTypes::GPR, OpTypes::GPR, OpTypes::simm13_lsb0,
    OpTypes::GPR, OpTypes::GPR, OpTypes::simm13_lsb0,
    OpTypes::GPR, OpTypes::GPR, OpTypes::simm13_lsb0,
    OpTypes::GPR, OpTypes::GPR, OpTypes::simm13_lsb0,
    OpTypes::GPR, OpTypes::csr_sysreg, OpTypes::GPR,
    OpTypes::GPR, OpTypes::csr_sysreg, OpTypes::uimm5,
    OpTypes::GPR, OpTypes::csr_sysreg, OpTypes::GPR,
    OpTypes::GPR, OpTypes::csr_sysreg, OpTypes::uimm5,
    OpTypes::GPR, OpTypes::csr_sysreg, OpTypes::GPR,
    OpTypes::GPR, OpTypes::csr_sysreg, OpTypes::uimm5,
    OpTypes::GPRNoX0, OpTypes::GPRNoX0, OpTypes::GPRNoX0,
    OpTypes::GPRNoX0, OpTypes::GPRNoX0, OpTypes::simm6nonzero,
    OpTypes::SP, OpTypes::SP, OpTypes::simm10_lsb0000nonzero,
    OpTypes::GPRC, OpTypes::SP, OpTypes::uimm10_lsb00nonzero,
    OpTypes::GPRNoX0, OpTypes::GPRNoX0, OpTypes::simm6,
    OpTypes::GPRNoX0, OpTypes::GPRNoX0, OpTypes::immzero,
    OpTypes::GPRX0, OpTypes::GPRX0, OpTypes::simm6nonzero,
    OpTypes::GPRX0, OpTypes::GPRX0, OpTypes::immzero,
    OpTypes::GPRC, OpTypes::GPRC, OpTypes::GPRC,
    OpTypes::GPRX0, OpTypes::GPRX0, OpTypes::GPRNoX0,
    OpTypes::GPRC, OpTypes::GPRC, OpTypes::GPRC,
    OpTypes::GPRC, OpTypes::GPRC, OpTypes::simm6,
    OpTypes::GPRC, OpTypes::simm9_lsb0,
    OpTypes::GPRC, OpTypes::simm9_lsb0,
    /**/
    OpTypes::FPR64C, OpTypes::GPRC, OpTypes::uimm8_lsb000,
    OpTypes::FPR64, OpTypes::SP, OpTypes::uimm9_lsb000,
    OpTypes::FPR32C, OpTypes::GPRC, OpTypes::uimm7_lsb00,
    OpTypes::FPR32, OpTypes::SP, OpTypes::uimm8_lsb00,
    OpTypes::FPR64C, OpTypes::GPRC, OpTypes::uimm8_lsb000,
    OpTypes::FPR64, OpTypes::SP, OpTypes::uimm9_lsb000,
    OpTypes::FPR32C, OpTypes::GPRC, OpTypes::uimm7_lsb00,
    OpTypes::FPR32, OpTypes::SP, OpTypes::uimm8_lsb00,
    OpTypes::simm12_lsb0,
    OpTypes::simm12_lsb0,
    OpTypes::GPRNoX0,
    OpTypes::GPRNoX0,
    OpTypes::GPRC, OpTypes::GPRC, OpTypes::uimm8_lsb000,
    OpTypes::GPRNoX0, OpTypes::SP, OpTypes::uimm9_lsb000,
    OpTypes::GPRNoX0, OpTypes::simm6,
    OpTypes::GPRX0, OpTypes::simm6,
    OpTypes::GPRNoX0X2, OpTypes::c_lui_imm,
    OpTypes::GPRX0, OpTypes::c_lui_imm,
    OpTypes::GPRC, OpTypes::GPRC, OpTypes::uimm7_lsb00,
    OpTypes::GPRNoX0, OpTypes::SP, OpTypes::uimm8_lsb00,
    OpTypes::GPRNoX0, OpTypes::GPRNoX0,
    OpTypes::GPRX0, OpTypes::GPRNoX0,
    /**/
    OpTypes::simm6nonzero,
    OpTypes::GPRC, OpTypes::GPRC, OpTypes::GPRC,
    OpTypes::GPRC, OpTypes::GPRC, OpTypes::uimm8_lsb000,
    OpTypes::GPR, OpTypes::SP, OpTypes::uimm9_lsb000,
    OpTypes::GPRNoX0, OpTypes::GPRNoX0, OpTypes::uimmlog2xlennonzero,
    OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPRX0, OpTypes::GPRX0, OpTypes::uimmlog2xlennonzero,
    OpTypes::GPRC, OpTypes::GPRC, OpTypes::uimmlog2xlennonzero,
    OpTypes::GPRC, OpTypes::GPRC,
    OpTypes::GPRC, OpTypes::GPRC, OpTypes::uimmlog2xlennonzero,
    OpTypes::GPRC, OpTypes::GPRC,
    OpTypes::GPRC, OpTypes::GPRC, OpTypes::GPRC,
    OpTypes::GPRC, OpTypes::GPRC, OpTypes::GPRC,
    OpTypes::GPRC, OpTypes::GPRC, OpTypes::uimm7_lsb00,
    OpTypes::GPR, OpTypes::SP, OpTypes::uimm8_lsb00,
    /**/
    OpTypes::GPRC, OpTypes::GPRC, OpTypes::GPRC,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR,
    /**/
    /**/
    OpTypes::FPR64, OpTypes::FPR64, OpTypes::FPR64, OpTypes::frmarg,
    OpTypes::FPR32, OpTypes::FPR32, OpTypes::FPR32, OpTypes::frmarg,
    OpTypes::GPR, OpTypes::FPR64,
    OpTypes::GPR, OpTypes::FPR32,
    OpTypes::FPR64, OpTypes::GPR, OpTypes::frmarg,
    OpTypes::FPR64, OpTypes::GPR, OpTypes::frmarg,
    OpTypes::FPR64, OpTypes::FPR32,
    OpTypes::FPR64, OpTypes::GPR,
    OpTypes::FPR64, OpTypes::GPR,
    OpTypes::GPR, OpTypes::FPR64, OpTypes::frmarg,
    OpTypes::GPR, OpTypes::FPR32, OpTypes::frmarg,
    OpTypes::GPR, OpTypes::FPR64, OpTypes::frmarg,
    OpTypes::GPR, OpTypes::FPR32, OpTypes::frmarg,
    OpTypes::FPR32, OpTypes::FPR64, OpTypes::frmarg,
    OpTypes::FPR32, OpTypes::GPR, OpTypes::frmarg,
    OpTypes::FPR32, OpTypes::GPR, OpTypes::frmarg,
    OpTypes::FPR32, OpTypes::GPR, OpTypes::frmarg,
    OpTypes::FPR32, OpTypes::GPR, OpTypes::frmarg,
    OpTypes::GPR, OpTypes::FPR64, OpTypes::frmarg,
    OpTypes::GPR, OpTypes::FPR32, OpTypes::frmarg,
    OpTypes::GPR, OpTypes::FPR64, OpTypes::frmarg,
    OpTypes::GPR, OpTypes::FPR32, OpTypes::frmarg,
    OpTypes::FPR64, OpTypes::FPR64, OpTypes::FPR64, OpTypes::frmarg,
    OpTypes::FPR32, OpTypes::FPR32, OpTypes::FPR32, OpTypes::frmarg,
    OpTypes::fencearg, OpTypes::fencearg,
    /**/
    /**/
    OpTypes::GPR, OpTypes::FPR64, OpTypes::FPR64,
    OpTypes::GPR, OpTypes::FPR32, OpTypes::FPR32,
    OpTypes::FPR64, OpTypes::GPR, OpTypes::simm12,
    OpTypes::GPR, OpTypes::FPR64, OpTypes::FPR64,
    OpTypes::GPR, OpTypes::FPR32, OpTypes::FPR32,
    OpTypes::GPR, OpTypes::FPR64, OpTypes::FPR64,
    OpTypes::GPR, OpTypes::FPR32, OpTypes::FPR32,
    OpTypes::FPR32, OpTypes::GPR, OpTypes::simm12,
    OpTypes::FPR64, OpTypes::FPR64, OpTypes::FPR64, OpTypes::FPR64, OpTypes::frmarg,
    OpTypes::FPR32, OpTypes::FPR32, OpTypes::FPR32, OpTypes::FPR32, OpTypes::frmarg,
    OpTypes::FPR64, OpTypes::FPR64, OpTypes::FPR64,
    OpTypes::FPR32, OpTypes::FPR32, OpTypes::FPR32,
    OpTypes::FPR64, OpTypes::FPR64, OpTypes::FPR64,
    OpTypes::FPR32, OpTypes::FPR32, OpTypes::FPR32,
    OpTypes::FPR64, OpTypes::FPR64, OpTypes::FPR64, OpTypes::FPR64, OpTypes::frmarg,
    OpTypes::FPR32, OpTypes::FPR32, OpTypes::FPR32, OpTypes::FPR32, OpTypes::frmarg,
    OpTypes::FPR64, OpTypes::FPR64, OpTypes::FPR64, OpTypes::frmarg,
    OpTypes::FPR32, OpTypes::FPR32, OpTypes::FPR32, OpTypes::frmarg,
    OpTypes::FPR64, OpTypes::GPR,
    OpTypes::FPR32, OpTypes::GPR,
    OpTypes::GPR, OpTypes::FPR64,
    OpTypes::GPR, OpTypes::FPR32,
    OpTypes::FPR64, OpTypes::FPR64, OpTypes::FPR64, OpTypes::FPR64, OpTypes::frmarg,
    OpTypes::FPR32, OpTypes::FPR32, OpTypes::FPR32, OpTypes::FPR32, OpTypes::frmarg,
    OpTypes::FPR64, OpTypes::FPR64, OpTypes::FPR64, OpTypes::FPR64, OpTypes::frmarg,
    OpTypes::FPR32, OpTypes::FPR32, OpTypes::FPR32, OpTypes::FPR32, OpTypes::frmarg,
    OpTypes::FPR64, OpTypes::GPR, OpTypes::simm12,
    OpTypes::FPR64, OpTypes::FPR64, OpTypes::FPR64,
    OpTypes::FPR32, OpTypes::FPR32, OpTypes::FPR32,
    OpTypes::FPR64, OpTypes::FPR64, OpTypes::FPR64,
    OpTypes::FPR32, OpTypes::FPR32, OpTypes::FPR32,
    OpTypes::FPR64, OpTypes::FPR64, OpTypes::FPR64,
    OpTypes::FPR32, OpTypes::FPR32, OpTypes::FPR32,
    OpTypes::FPR64, OpTypes::FPR64, OpTypes::frmarg,
    OpTypes::FPR32, OpTypes::FPR32, OpTypes::frmarg,
    OpTypes::FPR64, OpTypes::FPR64, OpTypes::FPR64, OpTypes::frmarg,
    OpTypes::FPR32, OpTypes::FPR32, OpTypes::FPR32, OpTypes::frmarg,
    OpTypes::FPR32, OpTypes::GPR, OpTypes::simm12,
    OpTypes::GPR, OpTypes::simm21_lsb0_jal,
    OpTypes::GPR, OpTypes::GPR, OpTypes::simm12,
    OpTypes::GPR, OpTypes::GPR, OpTypes::simm12,
    OpTypes::GPR, OpTypes::GPR, OpTypes::simm12,
    OpTypes::GPR, OpTypes::GPR, OpTypes::simm12,
    OpTypes::GPR, OpTypes::GPR, OpTypes::simm12,
    OpTypes::GPR, OpTypes::GPR, OpTypes::simm12,
    OpTypes::GPR, OpTypes::GPRMemAtomic,
    OpTypes::GPR, OpTypes::GPRMemAtomic,
    OpTypes::GPR, OpTypes::GPRMemAtomic,
    OpTypes::GPR, OpTypes::GPRMemAtomic,
    OpTypes::GPR, OpTypes::GPRMemAtomic,
    OpTypes::GPR, OpTypes::GPRMemAtomic,
    OpTypes::GPR, OpTypes::GPRMemAtomic,
    OpTypes::GPR, OpTypes::GPRMemAtomic,
    OpTypes::GPR, OpTypes::uimm20_lui,
    OpTypes::GPR, OpTypes::GPR, OpTypes::simm12,
    OpTypes::GPR, OpTypes::GPR, OpTypes::simm12,
    OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::simm12,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::simm12,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPRMemAtomic, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::simm12,
    OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::simm12,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::uimmlog2xlen,
    OpTypes::GPR, OpTypes::GPR, OpTypes::uimm5,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::simm12,
    OpTypes::GPR, OpTypes::GPR, OpTypes::simm12,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::uimmlog2xlen,
    OpTypes::GPR, OpTypes::GPR, OpTypes::uimm5,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::uimmlog2xlen,
    OpTypes::GPR, OpTypes::GPR, OpTypes::uimm5,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::simm12,
    /**/
    OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::GPR,
    OpTypes::GPR, OpTypes::GPR, OpTypes::simm12,
  };
  return OpcodeOperandTypes[Offsets[Opcode] + OpIdx];
}
} // end namespace RISCV
} // end namespace llvm
#endif // GET_INSTRINFO_OPERAND_TYPE

