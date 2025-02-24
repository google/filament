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

namespace LoongArch {
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
    DBG_VALUE_LIST	= 14,
    DBG_INSTR_REF	= 15,
    DBG_PHI	= 16,
    DBG_LABEL	= 17,
    REG_SEQUENCE	= 18,
    COPY	= 19,
    BUNDLE	= 20,
    LIFETIME_START	= 21,
    LIFETIME_END	= 22,
    PSEUDO_PROBE	= 23,
    ARITH_FENCE	= 24,
    STACKMAP	= 25,
    FENTRY_CALL	= 26,
    PATCHPOINT	= 27,
    LOAD_STACK_GUARD	= 28,
    PREALLOCATED_SETUP	= 29,
    PREALLOCATED_ARG	= 30,
    STATEPOINT	= 31,
    LOCAL_ESCAPE	= 32,
    FAULTING_OP	= 33,
    PATCHABLE_OP	= 34,
    PATCHABLE_FUNCTION_ENTER	= 35,
    PATCHABLE_RET	= 36,
    PATCHABLE_FUNCTION_EXIT	= 37,
    PATCHABLE_TAIL_CALL	= 38,
    PATCHABLE_EVENT_CALL	= 39,
    PATCHABLE_TYPED_EVENT_CALL	= 40,
    ICALL_BRANCH_FUNNEL	= 41,
    MEMBARRIER	= 42,
    G_ASSERT_SEXT	= 43,
    G_ASSERT_ZEXT	= 44,
    G_ASSERT_ALIGN	= 45,
    G_ADD	= 46,
    G_SUB	= 47,
    G_MUL	= 48,
    G_SDIV	= 49,
    G_UDIV	= 50,
    G_SREM	= 51,
    G_UREM	= 52,
    G_SDIVREM	= 53,
    G_UDIVREM	= 54,
    G_AND	= 55,
    G_OR	= 56,
    G_XOR	= 57,
    G_IMPLICIT_DEF	= 58,
    G_PHI	= 59,
    G_FRAME_INDEX	= 60,
    G_GLOBAL_VALUE	= 61,
    G_EXTRACT	= 62,
    G_UNMERGE_VALUES	= 63,
    G_INSERT	= 64,
    G_MERGE_VALUES	= 65,
    G_BUILD_VECTOR	= 66,
    G_BUILD_VECTOR_TRUNC	= 67,
    G_CONCAT_VECTORS	= 68,
    G_PTRTOINT	= 69,
    G_INTTOPTR	= 70,
    G_BITCAST	= 71,
    G_FREEZE	= 72,
    G_INTRINSIC_FPTRUNC_ROUND	= 73,
    G_INTRINSIC_TRUNC	= 74,
    G_INTRINSIC_ROUND	= 75,
    G_INTRINSIC_LRINT	= 76,
    G_INTRINSIC_ROUNDEVEN	= 77,
    G_READCYCLECOUNTER	= 78,
    G_LOAD	= 79,
    G_SEXTLOAD	= 80,
    G_ZEXTLOAD	= 81,
    G_INDEXED_LOAD	= 82,
    G_INDEXED_SEXTLOAD	= 83,
    G_INDEXED_ZEXTLOAD	= 84,
    G_STORE	= 85,
    G_INDEXED_STORE	= 86,
    G_ATOMIC_CMPXCHG_WITH_SUCCESS	= 87,
    G_ATOMIC_CMPXCHG	= 88,
    G_ATOMICRMW_XCHG	= 89,
    G_ATOMICRMW_ADD	= 90,
    G_ATOMICRMW_SUB	= 91,
    G_ATOMICRMW_AND	= 92,
    G_ATOMICRMW_NAND	= 93,
    G_ATOMICRMW_OR	= 94,
    G_ATOMICRMW_XOR	= 95,
    G_ATOMICRMW_MAX	= 96,
    G_ATOMICRMW_MIN	= 97,
    G_ATOMICRMW_UMAX	= 98,
    G_ATOMICRMW_UMIN	= 99,
    G_ATOMICRMW_FADD	= 100,
    G_ATOMICRMW_FSUB	= 101,
    G_ATOMICRMW_FMAX	= 102,
    G_ATOMICRMW_FMIN	= 103,
    G_ATOMICRMW_UINC_WRAP	= 104,
    G_ATOMICRMW_UDEC_WRAP	= 105,
    G_FENCE	= 106,
    G_BRCOND	= 107,
    G_BRINDIRECT	= 108,
    G_INVOKE_REGION_START	= 109,
    G_INTRINSIC	= 110,
    G_INTRINSIC_W_SIDE_EFFECTS	= 111,
    G_ANYEXT	= 112,
    G_TRUNC	= 113,
    G_CONSTANT	= 114,
    G_FCONSTANT	= 115,
    G_VASTART	= 116,
    G_VAARG	= 117,
    G_SEXT	= 118,
    G_SEXT_INREG	= 119,
    G_ZEXT	= 120,
    G_SHL	= 121,
    G_LSHR	= 122,
    G_ASHR	= 123,
    G_FSHL	= 124,
    G_FSHR	= 125,
    G_ROTR	= 126,
    G_ROTL	= 127,
    G_ICMP	= 128,
    G_FCMP	= 129,
    G_SELECT	= 130,
    G_UADDO	= 131,
    G_UADDE	= 132,
    G_USUBO	= 133,
    G_USUBE	= 134,
    G_SADDO	= 135,
    G_SADDE	= 136,
    G_SSUBO	= 137,
    G_SSUBE	= 138,
    G_UMULO	= 139,
    G_SMULO	= 140,
    G_UMULH	= 141,
    G_SMULH	= 142,
    G_UADDSAT	= 143,
    G_SADDSAT	= 144,
    G_USUBSAT	= 145,
    G_SSUBSAT	= 146,
    G_USHLSAT	= 147,
    G_SSHLSAT	= 148,
    G_SMULFIX	= 149,
    G_UMULFIX	= 150,
    G_SMULFIXSAT	= 151,
    G_UMULFIXSAT	= 152,
    G_SDIVFIX	= 153,
    G_UDIVFIX	= 154,
    G_SDIVFIXSAT	= 155,
    G_UDIVFIXSAT	= 156,
    G_FADD	= 157,
    G_FSUB	= 158,
    G_FMUL	= 159,
    G_FMA	= 160,
    G_FMAD	= 161,
    G_FDIV	= 162,
    G_FREM	= 163,
    G_FPOW	= 164,
    G_FPOWI	= 165,
    G_FEXP	= 166,
    G_FEXP2	= 167,
    G_FLOG	= 168,
    G_FLOG2	= 169,
    G_FLOG10	= 170,
    G_FNEG	= 171,
    G_FPEXT	= 172,
    G_FPTRUNC	= 173,
    G_FPTOSI	= 174,
    G_FPTOUI	= 175,
    G_SITOFP	= 176,
    G_UITOFP	= 177,
    G_FABS	= 178,
    G_FCOPYSIGN	= 179,
    G_IS_FPCLASS	= 180,
    G_FCANONICALIZE	= 181,
    G_FMINNUM	= 182,
    G_FMAXNUM	= 183,
    G_FMINNUM_IEEE	= 184,
    G_FMAXNUM_IEEE	= 185,
    G_FMINIMUM	= 186,
    G_FMAXIMUM	= 187,
    G_PTR_ADD	= 188,
    G_PTRMASK	= 189,
    G_SMIN	= 190,
    G_SMAX	= 191,
    G_UMIN	= 192,
    G_UMAX	= 193,
    G_ABS	= 194,
    G_LROUND	= 195,
    G_LLROUND	= 196,
    G_BR	= 197,
    G_BRJT	= 198,
    G_INSERT_VECTOR_ELT	= 199,
    G_EXTRACT_VECTOR_ELT	= 200,
    G_SHUFFLE_VECTOR	= 201,
    G_CTTZ	= 202,
    G_CTTZ_ZERO_UNDEF	= 203,
    G_CTLZ	= 204,
    G_CTLZ_ZERO_UNDEF	= 205,
    G_CTPOP	= 206,
    G_BSWAP	= 207,
    G_BITREVERSE	= 208,
    G_FCEIL	= 209,
    G_FCOS	= 210,
    G_FSIN	= 211,
    G_FSQRT	= 212,
    G_FFLOOR	= 213,
    G_FRINT	= 214,
    G_FNEARBYINT	= 215,
    G_ADDRSPACE_CAST	= 216,
    G_BLOCK_ADDR	= 217,
    G_JUMP_TABLE	= 218,
    G_DYN_STACKALLOC	= 219,
    G_STRICT_FADD	= 220,
    G_STRICT_FSUB	= 221,
    G_STRICT_FMUL	= 222,
    G_STRICT_FDIV	= 223,
    G_STRICT_FREM	= 224,
    G_STRICT_FMA	= 225,
    G_STRICT_FSQRT	= 226,
    G_READ_REGISTER	= 227,
    G_WRITE_REGISTER	= 228,
    G_MEMCPY	= 229,
    G_MEMCPY_INLINE	= 230,
    G_MEMMOVE	= 231,
    G_MEMSET	= 232,
    G_BZERO	= 233,
    G_VECREDUCE_SEQ_FADD	= 234,
    G_VECREDUCE_SEQ_FMUL	= 235,
    G_VECREDUCE_FADD	= 236,
    G_VECREDUCE_FMUL	= 237,
    G_VECREDUCE_FMAX	= 238,
    G_VECREDUCE_FMIN	= 239,
    G_VECREDUCE_ADD	= 240,
    G_VECREDUCE_MUL	= 241,
    G_VECREDUCE_AND	= 242,
    G_VECREDUCE_OR	= 243,
    G_VECREDUCE_XOR	= 244,
    G_VECREDUCE_SMAX	= 245,
    G_VECREDUCE_SMIN	= 246,
    G_VECREDUCE_UMAX	= 247,
    G_VECREDUCE_UMIN	= 248,
    G_SBFX	= 249,
    G_UBFX	= 250,
    ADJCALLSTACKDOWN	= 251,
    ADJCALLSTACKUP	= 252,
    PseudoAtomicLoadAdd32	= 253,
    PseudoAtomicLoadAnd32	= 254,
    PseudoAtomicLoadNand32	= 255,
    PseudoAtomicLoadNand64	= 256,
    PseudoAtomicLoadOr32	= 257,
    PseudoAtomicLoadSub32	= 258,
    PseudoAtomicLoadXor32	= 259,
    PseudoAtomicStoreD	= 260,
    PseudoAtomicStoreW	= 261,
    PseudoAtomicSwap32	= 262,
    PseudoBR	= 263,
    PseudoBRIND	= 264,
    PseudoB_TAIL	= 265,
    PseudoCALL	= 266,
    PseudoCALLIndirect	= 267,
    PseudoCmpXchg32	= 268,
    PseudoCmpXchg64	= 269,
    PseudoJIRL_CALL	= 270,
    PseudoJIRL_TAIL	= 271,
    PseudoLA_ABS	= 272,
    PseudoLA_ABS_LARGE	= 273,
    PseudoLA_GOT	= 274,
    PseudoLA_GOT_LARGE	= 275,
    PseudoLA_PCREL	= 276,
    PseudoLA_PCREL_LARGE	= 277,
    PseudoLA_TLS_GD	= 278,
    PseudoLA_TLS_GD_LARGE	= 279,
    PseudoLA_TLS_IE	= 280,
    PseudoLA_TLS_IE_LARGE	= 281,
    PseudoLA_TLS_LD	= 282,
    PseudoLA_TLS_LD_LARGE	= 283,
    PseudoLA_TLS_LE	= 284,
    PseudoLD_CFR	= 285,
    PseudoLI_D	= 286,
    PseudoLI_W	= 287,
    PseudoMaskedAtomicLoadAdd32	= 288,
    PseudoMaskedAtomicLoadMax32	= 289,
    PseudoMaskedAtomicLoadMin32	= 290,
    PseudoMaskedAtomicLoadNand32	= 291,
    PseudoMaskedAtomicLoadSub32	= 292,
    PseudoMaskedAtomicLoadUMax32	= 293,
    PseudoMaskedAtomicLoadUMin32	= 294,
    PseudoMaskedAtomicSwap32	= 295,
    PseudoMaskedCmpXchg32	= 296,
    PseudoRET	= 297,
    PseudoST_CFR	= 298,
    PseudoTAIL	= 299,
    PseudoTAILIndirect	= 300,
    PseudoUNIMP	= 301,
    RDFCSR	= 302,
    WRFCSR	= 303,
    ADDI_D	= 304,
    ADDI_W	= 305,
    ADDU16I_D	= 306,
    ADD_D	= 307,
    ADD_W	= 308,
    ALSL_D	= 309,
    ALSL_W	= 310,
    ALSL_WU	= 311,
    AMADD_D	= 312,
    AMADD_DB_D	= 313,
    AMADD_DB_W	= 314,
    AMADD_W	= 315,
    AMAND_D	= 316,
    AMAND_DB_D	= 317,
    AMAND_DB_W	= 318,
    AMAND_W	= 319,
    AMMAX_D	= 320,
    AMMAX_DB_D	= 321,
    AMMAX_DB_DU	= 322,
    AMMAX_DB_W	= 323,
    AMMAX_DB_WU	= 324,
    AMMAX_DU	= 325,
    AMMAX_W	= 326,
    AMMAX_WU	= 327,
    AMMIN_D	= 328,
    AMMIN_DB_D	= 329,
    AMMIN_DB_DU	= 330,
    AMMIN_DB_W	= 331,
    AMMIN_DB_WU	= 332,
    AMMIN_DU	= 333,
    AMMIN_W	= 334,
    AMMIN_WU	= 335,
    AMOR_D	= 336,
    AMOR_DB_D	= 337,
    AMOR_DB_W	= 338,
    AMOR_W	= 339,
    AMSWAP_D	= 340,
    AMSWAP_DB_D	= 341,
    AMSWAP_DB_W	= 342,
    AMSWAP_W	= 343,
    AMXOR_D	= 344,
    AMXOR_DB_D	= 345,
    AMXOR_DB_W	= 346,
    AMXOR_W	= 347,
    AND	= 348,
    ANDI	= 349,
    ANDN	= 350,
    ASRTGT_D	= 351,
    ASRTLE_D	= 352,
    B	= 353,
    BCEQZ	= 354,
    BCNEZ	= 355,
    BEQ	= 356,
    BEQZ	= 357,
    BGE	= 358,
    BGEU	= 359,
    BITREV_4B	= 360,
    BITREV_8B	= 361,
    BITREV_D	= 362,
    BITREV_W	= 363,
    BL	= 364,
    BLT	= 365,
    BLTU	= 366,
    BNE	= 367,
    BNEZ	= 368,
    BREAK	= 369,
    BSTRINS_D	= 370,
    BSTRINS_W	= 371,
    BSTRPICK_D	= 372,
    BSTRPICK_W	= 373,
    BYTEPICK_D	= 374,
    BYTEPICK_W	= 375,
    CACOP	= 376,
    CLO_D	= 377,
    CLO_W	= 378,
    CLZ_D	= 379,
    CLZ_W	= 380,
    CPUCFG	= 381,
    CRCC_W_B_W	= 382,
    CRCC_W_D_W	= 383,
    CRCC_W_H_W	= 384,
    CRCC_W_W_W	= 385,
    CRC_W_B_W	= 386,
    CRC_W_D_W	= 387,
    CRC_W_H_W	= 388,
    CRC_W_W_W	= 389,
    CSRRD	= 390,
    CSRWR	= 391,
    CSRXCHG	= 392,
    CTO_D	= 393,
    CTO_W	= 394,
    CTZ_D	= 395,
    CTZ_W	= 396,
    DBAR	= 397,
    DBCL	= 398,
    DIV_D	= 399,
    DIV_DU	= 400,
    DIV_W	= 401,
    DIV_WU	= 402,
    ERTN	= 403,
    EXT_W_B	= 404,
    EXT_W_H	= 405,
    FABS_D	= 406,
    FABS_S	= 407,
    FADD_D	= 408,
    FADD_S	= 409,
    FCLASS_D	= 410,
    FCLASS_S	= 411,
    FCMP_CAF_D	= 412,
    FCMP_CAF_S	= 413,
    FCMP_CEQ_D	= 414,
    FCMP_CEQ_S	= 415,
    FCMP_CLE_D	= 416,
    FCMP_CLE_S	= 417,
    FCMP_CLT_D	= 418,
    FCMP_CLT_S	= 419,
    FCMP_CNE_D	= 420,
    FCMP_CNE_S	= 421,
    FCMP_COR_D	= 422,
    FCMP_COR_S	= 423,
    FCMP_CUEQ_D	= 424,
    FCMP_CUEQ_S	= 425,
    FCMP_CULE_D	= 426,
    FCMP_CULE_S	= 427,
    FCMP_CULT_D	= 428,
    FCMP_CULT_S	= 429,
    FCMP_CUNE_D	= 430,
    FCMP_CUNE_S	= 431,
    FCMP_CUN_D	= 432,
    FCMP_CUN_S	= 433,
    FCMP_SAF_D	= 434,
    FCMP_SAF_S	= 435,
    FCMP_SEQ_D	= 436,
    FCMP_SEQ_S	= 437,
    FCMP_SLE_D	= 438,
    FCMP_SLE_S	= 439,
    FCMP_SLT_D	= 440,
    FCMP_SLT_S	= 441,
    FCMP_SNE_D	= 442,
    FCMP_SNE_S	= 443,
    FCMP_SOR_D	= 444,
    FCMP_SOR_S	= 445,
    FCMP_SUEQ_D	= 446,
    FCMP_SUEQ_S	= 447,
    FCMP_SULE_D	= 448,
    FCMP_SULE_S	= 449,
    FCMP_SULT_D	= 450,
    FCMP_SULT_S	= 451,
    FCMP_SUNE_D	= 452,
    FCMP_SUNE_S	= 453,
    FCMP_SUN_D	= 454,
    FCMP_SUN_S	= 455,
    FCOPYSIGN_D	= 456,
    FCOPYSIGN_S	= 457,
    FCVT_D_S	= 458,
    FCVT_S_D	= 459,
    FDIV_D	= 460,
    FDIV_S	= 461,
    FFINT_D_L	= 462,
    FFINT_D_W	= 463,
    FFINT_S_L	= 464,
    FFINT_S_W	= 465,
    FLDGT_D	= 466,
    FLDGT_S	= 467,
    FLDLE_D	= 468,
    FLDLE_S	= 469,
    FLDX_D	= 470,
    FLDX_S	= 471,
    FLD_D	= 472,
    FLD_S	= 473,
    FLOGB_D	= 474,
    FLOGB_S	= 475,
    FMADD_D	= 476,
    FMADD_S	= 477,
    FMAXA_D	= 478,
    FMAXA_S	= 479,
    FMAX_D	= 480,
    FMAX_S	= 481,
    FMINA_D	= 482,
    FMINA_S	= 483,
    FMIN_D	= 484,
    FMIN_S	= 485,
    FMOV_D	= 486,
    FMOV_S	= 487,
    FMSUB_D	= 488,
    FMSUB_S	= 489,
    FMUL_D	= 490,
    FMUL_S	= 491,
    FNEG_D	= 492,
    FNEG_S	= 493,
    FNMADD_D	= 494,
    FNMADD_S	= 495,
    FNMSUB_D	= 496,
    FNMSUB_S	= 497,
    FRECIP_D	= 498,
    FRECIP_S	= 499,
    FRINT_D	= 500,
    FRINT_S	= 501,
    FRSQRT_D	= 502,
    FRSQRT_S	= 503,
    FSCALEB_D	= 504,
    FSCALEB_S	= 505,
    FSEL_D	= 506,
    FSEL_S	= 507,
    FSQRT_D	= 508,
    FSQRT_S	= 509,
    FSTGT_D	= 510,
    FSTGT_S	= 511,
    FSTLE_D	= 512,
    FSTLE_S	= 513,
    FSTX_D	= 514,
    FSTX_S	= 515,
    FST_D	= 516,
    FST_S	= 517,
    FSUB_D	= 518,
    FSUB_S	= 519,
    FTINTRM_L_D	= 520,
    FTINTRM_L_S	= 521,
    FTINTRM_W_D	= 522,
    FTINTRM_W_S	= 523,
    FTINTRNE_L_D	= 524,
    FTINTRNE_L_S	= 525,
    FTINTRNE_W_D	= 526,
    FTINTRNE_W_S	= 527,
    FTINTRP_L_D	= 528,
    FTINTRP_L_S	= 529,
    FTINTRP_W_D	= 530,
    FTINTRP_W_S	= 531,
    FTINTRZ_L_D	= 532,
    FTINTRZ_L_S	= 533,
    FTINTRZ_W_D	= 534,
    FTINTRZ_W_S	= 535,
    FTINT_L_D	= 536,
    FTINT_L_S	= 537,
    FTINT_W_D	= 538,
    FTINT_W_S	= 539,
    IBAR	= 540,
    IDLE	= 541,
    INVTLB	= 542,
    IOCSRRD_B	= 543,
    IOCSRRD_D	= 544,
    IOCSRRD_H	= 545,
    IOCSRRD_W	= 546,
    IOCSRWR_B	= 547,
    IOCSRWR_D	= 548,
    IOCSRWR_H	= 549,
    IOCSRWR_W	= 550,
    JIRL	= 551,
    LDDIR	= 552,
    LDGT_B	= 553,
    LDGT_D	= 554,
    LDGT_H	= 555,
    LDGT_W	= 556,
    LDLE_B	= 557,
    LDLE_D	= 558,
    LDLE_H	= 559,
    LDLE_W	= 560,
    LDPTE	= 561,
    LDPTR_D	= 562,
    LDPTR_W	= 563,
    LDX_B	= 564,
    LDX_BU	= 565,
    LDX_D	= 566,
    LDX_H	= 567,
    LDX_HU	= 568,
    LDX_W	= 569,
    LDX_WU	= 570,
    LD_B	= 571,
    LD_BU	= 572,
    LD_D	= 573,
    LD_H	= 574,
    LD_HU	= 575,
    LD_W	= 576,
    LD_WU	= 577,
    LL_D	= 578,
    LL_W	= 579,
    LU12I_W	= 580,
    LU32I_D	= 581,
    LU52I_D	= 582,
    MASKEQZ	= 583,
    MASKNEZ	= 584,
    MOD_D	= 585,
    MOD_DU	= 586,
    MOD_W	= 587,
    MOD_WU	= 588,
    MOVCF2FR_S	= 589,
    MOVCF2GR	= 590,
    MOVFCSR2GR	= 591,
    MOVFR2CF_S	= 592,
    MOVFR2GR_D	= 593,
    MOVFR2GR_S	= 594,
    MOVFR2GR_S_64	= 595,
    MOVFRH2GR_S	= 596,
    MOVGR2CF	= 597,
    MOVGR2FCSR	= 598,
    MOVGR2FRH_W	= 599,
    MOVGR2FR_D	= 600,
    MOVGR2FR_W	= 601,
    MOVGR2FR_W_64	= 602,
    MULH_D	= 603,
    MULH_DU	= 604,
    MULH_W	= 605,
    MULH_WU	= 606,
    MULW_D_W	= 607,
    MULW_D_WU	= 608,
    MUL_D	= 609,
    MUL_W	= 610,
    NOR	= 611,
    OR	= 612,
    ORI	= 613,
    ORN	= 614,
    PCADDI	= 615,
    PCADDU12I	= 616,
    PCADDU18I	= 617,
    PCALAU12I	= 618,
    PRELD	= 619,
    PRELDX	= 620,
    RDTIMEH_W	= 621,
    RDTIMEL_W	= 622,
    RDTIME_D	= 623,
    REVB_2H	= 624,
    REVB_2W	= 625,
    REVB_4H	= 626,
    REVB_D	= 627,
    REVH_2W	= 628,
    REVH_D	= 629,
    ROTRI_D	= 630,
    ROTRI_W	= 631,
    ROTR_D	= 632,
    ROTR_W	= 633,
    SC_D	= 634,
    SC_W	= 635,
    SLLI_D	= 636,
    SLLI_W	= 637,
    SLL_D	= 638,
    SLL_W	= 639,
    SLT	= 640,
    SLTI	= 641,
    SLTU	= 642,
    SLTUI	= 643,
    SRAI_D	= 644,
    SRAI_W	= 645,
    SRA_D	= 646,
    SRA_W	= 647,
    SRLI_D	= 648,
    SRLI_W	= 649,
    SRL_D	= 650,
    SRL_W	= 651,
    STGT_B	= 652,
    STGT_D	= 653,
    STGT_H	= 654,
    STGT_W	= 655,
    STLE_B	= 656,
    STLE_D	= 657,
    STLE_H	= 658,
    STLE_W	= 659,
    STPTR_D	= 660,
    STPTR_W	= 661,
    STX_B	= 662,
    STX_D	= 663,
    STX_H	= 664,
    STX_W	= 665,
    ST_B	= 666,
    ST_D	= 667,
    ST_H	= 668,
    ST_W	= 669,
    SUB_D	= 670,
    SUB_W	= 671,
    SYSCALL	= 672,
    TLBCLR	= 673,
    TLBFILL	= 674,
    TLBFLUSH	= 675,
    TLBRD	= 676,
    TLBSRCH	= 677,
    TLBWR	= 678,
    XOR	= 679,
    XORI	= 680,
    INSTRUCTION_LIST_END = 681
  };

} // end namespace LoongArch
} // end namespace llvm
#endif // GET_INSTRINFO_ENUM

#ifdef GET_INSTRINFO_SCHED_ENUM
#undef GET_INSTRINFO_SCHED_ENUM
namespace llvm {

namespace LoongArch {
namespace Sched {
  enum {
    NoInstrModel	= 0,
    SCHED_LIST_END = 1
  };
} // end namespace Sched
} // end namespace LoongArch
} // end namespace llvm
#endif // GET_INSTRINFO_SCHED_ENUM

#ifdef GET_INSTRINFO_MC_DESC
#undef GET_INSTRINFO_MC_DESC
namespace llvm {

static const MCPhysReg ImplicitList1[] = { LoongArch::R3, LoongArch::R3 };
static const MCPhysReg ImplicitList2[] = { LoongArch::R3 };
static const MCPhysReg ImplicitList3[] = { LoongArch::R1 };

static const MCOperandInfo OperandInfo2[] = { { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo3[] = { { -1, 0, MCOI::OPERAND_IMMEDIATE, 0 }, };
static const MCOperandInfo OperandInfo4[] = { { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_IMMEDIATE, 0 }, };
static const MCOperandInfo OperandInfo5[] = { { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, MCOI_TIED_TO(0) }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_IMMEDIATE, 0 }, };
static const MCOperandInfo OperandInfo6[] = { { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_IMMEDIATE, 0 }, };
static const MCOperandInfo OperandInfo7[] = { { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo8[] = { { -1, 0, MCOI::OPERAND_IMMEDIATE, 0 }, { -1, 0, MCOI::OPERAND_IMMEDIATE, 0 }, { -1, 0, MCOI::OPERAND_IMMEDIATE, 0 }, { -1, 0, MCOI::OPERAND_IMMEDIATE, 0 }, };
static const MCOperandInfo OperandInfo9[] = { { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, MCOI_TIED_TO(0) }, };
static const MCOperandInfo OperandInfo10[] = { { -1, 0, MCOI::OPERAND_IMMEDIATE, 0 }, { -1, 0, MCOI::OPERAND_IMMEDIATE, 0 }, };
static const MCOperandInfo OperandInfo11[] = { { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_IMMEDIATE, 0 }, { -1, 0, MCOI::OPERAND_IMMEDIATE, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_IMMEDIATE, 0 }, { -1, 0, MCOI::OPERAND_IMMEDIATE, 0 }, };
static const MCOperandInfo OperandInfo12[] = { { 0, 0|(1<<MCOI::LookupPtrRegClass), MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo13[] = { { 0, 0|(1<<MCOI::LookupPtrRegClass), MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_IMMEDIATE, 0 }, { -1, 0, MCOI::OPERAND_IMMEDIATE, 0 }, };
static const MCOperandInfo OperandInfo14[] = { { 0, 0|(1<<MCOI::LookupPtrRegClass), MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_IMMEDIATE, 0 }, };
static const MCOperandInfo OperandInfo15[] = { { 0, 0|(1<<MCOI::LookupPtrRegClass), MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo16[] = { { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { 0, 0|(1<<MCOI::LookupPtrRegClass), MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo17[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_IMM_0, 0 }, };
static const MCOperandInfo OperandInfo18[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, };
static const MCOperandInfo OperandInfo19[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, };
static const MCOperandInfo OperandInfo20[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, };
static const MCOperandInfo OperandInfo21[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo22[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_IMM_0, 0 }, };
static const MCOperandInfo OperandInfo23[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, };
static const MCOperandInfo OperandInfo24[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_IMM_0, 0 }, };
static const MCOperandInfo OperandInfo25[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, };
static const MCOperandInfo OperandInfo26[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_IMMEDIATE, 0 }, };
static const MCOperandInfo OperandInfo27[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_2, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo28[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_2, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo29[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_2, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, };
static const MCOperandInfo OperandInfo30[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, };
static const MCOperandInfo OperandInfo31[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, };
static const MCOperandInfo OperandInfo32[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo33[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, };
static const MCOperandInfo OperandInfo34[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, };
static const MCOperandInfo OperandInfo35[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, };
static const MCOperandInfo OperandInfo36[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, };
static const MCOperandInfo OperandInfo37[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_IMM_0, 0 }, };
static const MCOperandInfo OperandInfo38[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, };
static const MCOperandInfo OperandInfo39[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_2, 0 }, };
static const MCOperandInfo OperandInfo40[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_2, 0 }, };
static const MCOperandInfo OperandInfo41[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo42[] = { { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, };
static const MCOperandInfo OperandInfo43[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_2, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_IMM_0, 0 }, };
static const MCOperandInfo OperandInfo44[] = { { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_0, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, { -1, 0, MCOI::OPERAND_GENERIC_1, 0 }, };
static const MCOperandInfo OperandInfo45[] = { { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, MCOI_EARLY_CLOBBER }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, MCOI_EARLY_CLOBBER }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo46[] = { { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo47[] = { { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo48[] = { { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo49[] = { { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, MCOI_EARLY_CLOBBER }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, MCOI_EARLY_CLOBBER }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo50[] = { { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo51[] = { { LoongArch::CFRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo52[] = { { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, MCOI_EARLY_CLOBBER }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, MCOI_EARLY_CLOBBER }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo53[] = { { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, MCOI_EARLY_CLOBBER }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, MCOI_EARLY_CLOBBER }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, MCOI_EARLY_CLOBBER }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo54[] = { { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, MCOI_EARLY_CLOBBER }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, MCOI_EARLY_CLOBBER }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, MCOI_EARLY_CLOBBER }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo55[] = { { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, MCOI_EARLY_CLOBBER }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, MCOI_EARLY_CLOBBER }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo56[] = { { LoongArch::GPRTRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo57[] = { { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo58[] = { { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo59[] = { { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, MCOI_EARLY_CLOBBER }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo60[] = { { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo61[] = { { LoongArch::CFRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo62[] = { { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, MCOI_TIED_TO(0) }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo63[] = { { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo64[] = { { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo65[] = { { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, MCOI_TIED_TO(0) }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo66[] = { { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, MCOI_TIED_TO(0) }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo67[] = { { LoongArch::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo68[] = { { LoongArch::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo69[] = { { LoongArch::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo70[] = { { LoongArch::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo71[] = { { LoongArch::CFRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo72[] = { { LoongArch::CFRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo73[] = { { LoongArch::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo74[] = { { LoongArch::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo75[] = { { LoongArch::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo76[] = { { LoongArch::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo77[] = { { LoongArch::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo78[] = { { LoongArch::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, };
static const MCOperandInfo OperandInfo79[] = { { LoongArch::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo80[] = { { LoongArch::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo81[] = { { LoongArch::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::CFRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo82[] = { { LoongArch::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::CFRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo83[] = { { LoongArch::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::CFRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo84[] = { { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::CFRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo85[] = { { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::FCSRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo86[] = { { LoongArch::CFRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo87[] = { { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo88[] = { { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo89[] = { { LoongArch::CFRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo90[] = { { LoongArch::FCSRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo91[] = { { LoongArch::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, MCOI_TIED_TO(0) }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo92[] = { { LoongArch::FPR64RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo93[] = { { LoongArch::FPR32RegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
static const MCOperandInfo OperandInfo94[] = { { -1, 0, MCOI::OPERAND_UNKNOWN, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { LoongArch::GPRRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };

extern const MCInstrDesc LoongArchInsts[] = {
  { 680,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo50 },  // Inst #680 = XORI
  { 679,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #679 = XOR
  { 678,	0,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr },  // Inst #678 = TLBWR
  { 677,	0,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr },  // Inst #677 = TLBSRCH
  { 676,	0,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr },  // Inst #676 = TLBRD
  { 675,	0,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr },  // Inst #675 = TLBFLUSH
  { 674,	0,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr },  // Inst #674 = TLBFILL
  { 673,	0,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr },  // Inst #673 = TLBCLR
  { 672,	1,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo2 },  // Inst #672 = SYSCALL
  { 671,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #671 = SUB_W
  { 670,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #670 = SUB_D
  { 669,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo50 },  // Inst #669 = ST_W
  { 668,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo50 },  // Inst #668 = ST_H
  { 667,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo50 },  // Inst #667 = ST_D
  { 666,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo50 },  // Inst #666 = ST_B
  { 665,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo46 },  // Inst #665 = STX_W
  { 664,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo46 },  // Inst #664 = STX_H
  { 663,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo46 },  // Inst #663 = STX_D
  { 662,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo46 },  // Inst #662 = STX_B
  { 661,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo50 },  // Inst #661 = STPTR_W
  { 660,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo50 },  // Inst #660 = STPTR_D
  { 659,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo46 },  // Inst #659 = STLE_W
  { 658,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo46 },  // Inst #658 = STLE_H
  { 657,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo46 },  // Inst #657 = STLE_D
  { 656,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo46 },  // Inst #656 = STLE_B
  { 655,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo46 },  // Inst #655 = STGT_W
  { 654,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo46 },  // Inst #654 = STGT_H
  { 653,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo46 },  // Inst #653 = STGT_D
  { 652,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo46 },  // Inst #652 = STGT_B
  { 651,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #651 = SRL_W
  { 650,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #650 = SRL_D
  { 649,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo50 },  // Inst #649 = SRLI_W
  { 648,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo50 },  // Inst #648 = SRLI_D
  { 647,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #647 = SRA_W
  { 646,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #646 = SRA_D
  { 645,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo50 },  // Inst #645 = SRAI_W
  { 644,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo50 },  // Inst #644 = SRAI_D
  { 643,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo50 },  // Inst #643 = SLTUI
  { 642,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #642 = SLTU
  { 641,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo50 },  // Inst #641 = SLTI
  { 640,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #640 = SLT
  { 639,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #639 = SLL_W
  { 638,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #638 = SLL_D
  { 637,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo50 },  // Inst #637 = SLLI_W
  { 636,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo50 },  // Inst #636 = SLLI_D
  { 635,	4,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo66 },  // Inst #635 = SC_W
  { 634,	4,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo66 },  // Inst #634 = SC_D
  { 633,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #633 = ROTR_W
  { 632,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #632 = ROTR_D
  { 631,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo50 },  // Inst #631 = ROTRI_W
  { 630,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo50 },  // Inst #630 = ROTRI_D
  { 629,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo60 },  // Inst #629 = REVH_D
  { 628,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo60 },  // Inst #628 = REVH_2W
  { 627,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo60 },  // Inst #627 = REVB_D
  { 626,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo60 },  // Inst #626 = REVB_4H
  { 625,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo60 },  // Inst #625 = REVB_2W
  { 624,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo60 },  // Inst #624 = REVB_2H
  { 623,	2,	2,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo60 },  // Inst #623 = RDTIME_D
  { 622,	2,	2,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo60 },  // Inst #622 = RDTIMEL_W
  { 621,	2,	2,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo60 },  // Inst #621 = RDTIMEH_W
  { 620,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo94 },  // Inst #620 = PRELDX
  { 619,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo64 },  // Inst #619 = PRELD
  { 618,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo47 },  // Inst #618 = PCALAU12I
  { 617,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo47 },  // Inst #617 = PCADDU18I
  { 616,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo47 },  // Inst #616 = PCADDU12I
  { 615,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo47 },  // Inst #615 = PCADDI
  { 614,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #614 = ORN
  { 613,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo50 },  // Inst #613 = ORI
  { 612,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #612 = OR
  { 611,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #611 = NOR
  { 610,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #610 = MUL_W
  { 609,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #609 = MUL_D
  { 608,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #608 = MULW_D_WU
  { 607,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #607 = MULW_D_W
  { 606,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #606 = MULH_WU
  { 605,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #605 = MULH_W
  { 604,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #604 = MULH_DU
  { 603,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #603 = MULH_D
  { 602,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo92 },  // Inst #602 = MOVGR2FR_W_64
  { 601,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo93 },  // Inst #601 = MOVGR2FR_W
  { 600,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo92 },  // Inst #600 = MOVGR2FR_D
  { 599,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo91 },  // Inst #599 = MOVGR2FRH_W
  { 598,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo90 },  // Inst #598 = MOVGR2FCSR
  { 597,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo89 },  // Inst #597 = MOVGR2CF
  { 596,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo87 },  // Inst #596 = MOVFRH2GR_S
  { 595,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo87 },  // Inst #595 = MOVFR2GR_S_64
  { 594,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo88 },  // Inst #594 = MOVFR2GR_S
  { 593,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo87 },  // Inst #593 = MOVFR2GR_D
  { 592,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo86 },  // Inst #592 = MOVFR2CF_S
  { 591,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo85 },  // Inst #591 = MOVFCSR2GR
  { 590,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo84 },  // Inst #590 = MOVCF2GR
  { 589,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo83 },  // Inst #589 = MOVCF2FR_S
  { 588,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UsesCustomInserter), 0x0ULL, nullptr, OperandInfo46 },  // Inst #588 = MOD_WU
  { 587,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UsesCustomInserter), 0x0ULL, nullptr, OperandInfo46 },  // Inst #587 = MOD_W
  { 586,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UsesCustomInserter), 0x0ULL, nullptr, OperandInfo46 },  // Inst #586 = MOD_DU
  { 585,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UsesCustomInserter), 0x0ULL, nullptr, OperandInfo46 },  // Inst #585 = MOD_D
  { 584,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo46 },  // Inst #584 = MASKNEZ
  { 583,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo46 },  // Inst #583 = MASKEQZ
  { 582,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo50 },  // Inst #582 = LU52I_D
  { 581,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo65 },  // Inst #581 = LU32I_D
  { 580,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo47 },  // Inst #580 = LU12I_W
  { 579,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo50 },  // Inst #579 = LL_W
  { 578,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo50 },  // Inst #578 = LL_D
  { 577,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo50 },  // Inst #577 = LD_WU
  { 576,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo50 },  // Inst #576 = LD_W
  { 575,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo50 },  // Inst #575 = LD_HU
  { 574,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo50 },  // Inst #574 = LD_H
  { 573,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo50 },  // Inst #573 = LD_D
  { 572,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo50 },  // Inst #572 = LD_BU
  { 571,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo50 },  // Inst #571 = LD_B
  { 570,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo46 },  // Inst #570 = LDX_WU
  { 569,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo46 },  // Inst #569 = LDX_W
  { 568,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo46 },  // Inst #568 = LDX_HU
  { 567,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo46 },  // Inst #567 = LDX_H
  { 566,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo46 },  // Inst #566 = LDX_D
  { 565,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo46 },  // Inst #565 = LDX_BU
  { 564,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo46 },  // Inst #564 = LDX_B
  { 563,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo50 },  // Inst #563 = LDPTR_W
  { 562,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo50 },  // Inst #562 = LDPTR_D
  { 561,	2,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo47 },  // Inst #561 = LDPTE
  { 560,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo46 },  // Inst #560 = LDLE_W
  { 559,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo46 },  // Inst #559 = LDLE_H
  { 558,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo46 },  // Inst #558 = LDLE_D
  { 557,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo46 },  // Inst #557 = LDLE_B
  { 556,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo46 },  // Inst #556 = LDGT_W
  { 555,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo46 },  // Inst #555 = LDGT_H
  { 554,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo46 },  // Inst #554 = LDGT_D
  { 553,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo46 },  // Inst #553 = LDGT_B
  { 552,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo50 },  // Inst #552 = LDDIR
  { 551,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo50 },  // Inst #551 = JIRL
  { 550,	2,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo60 },  // Inst #550 = IOCSRWR_W
  { 549,	2,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo60 },  // Inst #549 = IOCSRWR_H
  { 548,	2,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo60 },  // Inst #548 = IOCSRWR_D
  { 547,	2,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo60 },  // Inst #547 = IOCSRWR_B
  { 546,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo60 },  // Inst #546 = IOCSRRD_W
  { 545,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo60 },  // Inst #545 = IOCSRRD_H
  { 544,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo60 },  // Inst #544 = IOCSRRD_D
  { 543,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo60 },  // Inst #543 = IOCSRRD_B
  { 542,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo50 },  // Inst #542 = INVTLB
  { 541,	1,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo2 },  // Inst #541 = IDLE
  { 540,	1,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo2 },  // Inst #540 = IBAR
  { 539,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo68 },  // Inst #539 = FTINT_W_S
  { 538,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo74 },  // Inst #538 = FTINT_W_D
  { 537,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo73 },  // Inst #537 = FTINT_L_S
  { 536,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo67 },  // Inst #536 = FTINT_L_D
  { 535,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo68 },  // Inst #535 = FTINTRZ_W_S
  { 534,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo74 },  // Inst #534 = FTINTRZ_W_D
  { 533,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo73 },  // Inst #533 = FTINTRZ_L_S
  { 532,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo67 },  // Inst #532 = FTINTRZ_L_D
  { 531,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo68 },  // Inst #531 = FTINTRP_W_S
  { 530,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo74 },  // Inst #530 = FTINTRP_W_D
  { 529,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo73 },  // Inst #529 = FTINTRP_L_S
  { 528,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo67 },  // Inst #528 = FTINTRP_L_D
  { 527,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo68 },  // Inst #527 = FTINTRNE_W_S
  { 526,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo74 },  // Inst #526 = FTINTRNE_W_D
  { 525,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo73 },  // Inst #525 = FTINTRNE_L_S
  { 524,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo67 },  // Inst #524 = FTINTRNE_L_D
  { 523,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo68 },  // Inst #523 = FTINTRM_W_S
  { 522,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo74 },  // Inst #522 = FTINTRM_W_D
  { 521,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo73 },  // Inst #521 = FTINTRM_L_S
  { 520,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo67 },  // Inst #520 = FTINTRM_L_D
  { 519,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo70 },  // Inst #519 = FSUB_S
  { 518,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo69 },  // Inst #518 = FSUB_D
  { 517,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo78 },  // Inst #517 = FST_S
  { 516,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo77 },  // Inst #516 = FST_D
  { 515,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo76 },  // Inst #515 = FSTX_S
  { 514,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo75 },  // Inst #514 = FSTX_D
  { 513,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo76 },  // Inst #513 = FSTLE_S
  { 512,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo75 },  // Inst #512 = FSTLE_D
  { 511,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo76 },  // Inst #511 = FSTGT_S
  { 510,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo75 },  // Inst #510 = FSTGT_D
  { 509,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo68 },  // Inst #509 = FSQRT_S
  { 508,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo67 },  // Inst #508 = FSQRT_D
  { 507,	4,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo82 },  // Inst #507 = FSEL_S
  { 506,	4,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo81 },  // Inst #506 = FSEL_D
  { 505,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo70 },  // Inst #505 = FSCALEB_S
  { 504,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo69 },  // Inst #504 = FSCALEB_D
  { 503,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo68 },  // Inst #503 = FRSQRT_S
  { 502,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo67 },  // Inst #502 = FRSQRT_D
  { 501,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo68 },  // Inst #501 = FRINT_S
  { 500,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo67 },  // Inst #500 = FRINT_D
  { 499,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo68 },  // Inst #499 = FRECIP_S
  { 498,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo67 },  // Inst #498 = FRECIP_D
  { 497,	4,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo80 },  // Inst #497 = FNMSUB_S
  { 496,	4,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo79 },  // Inst #496 = FNMSUB_D
  { 495,	4,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo80 },  // Inst #495 = FNMADD_S
  { 494,	4,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo79 },  // Inst #494 = FNMADD_D
  { 493,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo68 },  // Inst #493 = FNEG_S
  { 492,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo67 },  // Inst #492 = FNEG_D
  { 491,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo70 },  // Inst #491 = FMUL_S
  { 490,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo69 },  // Inst #490 = FMUL_D
  { 489,	4,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo80 },  // Inst #489 = FMSUB_S
  { 488,	4,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo79 },  // Inst #488 = FMSUB_D
  { 487,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo68 },  // Inst #487 = FMOV_S
  { 486,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo67 },  // Inst #486 = FMOV_D
  { 485,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo70 },  // Inst #485 = FMIN_S
  { 484,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo69 },  // Inst #484 = FMIN_D
  { 483,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo70 },  // Inst #483 = FMINA_S
  { 482,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo69 },  // Inst #482 = FMINA_D
  { 481,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo70 },  // Inst #481 = FMAX_S
  { 480,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo69 },  // Inst #480 = FMAX_D
  { 479,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo70 },  // Inst #479 = FMAXA_S
  { 478,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo69 },  // Inst #478 = FMAXA_D
  { 477,	4,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo80 },  // Inst #477 = FMADD_S
  { 476,	4,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo79 },  // Inst #476 = FMADD_D
  { 475,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo68 },  // Inst #475 = FLOGB_S
  { 474,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo67 },  // Inst #474 = FLOGB_D
  { 473,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo78 },  // Inst #473 = FLD_S
  { 472,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo77 },  // Inst #472 = FLD_D
  { 471,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo76 },  // Inst #471 = FLDX_S
  { 470,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo75 },  // Inst #470 = FLDX_D
  { 469,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo76 },  // Inst #469 = FLDLE_S
  { 468,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo75 },  // Inst #468 = FLDLE_D
  { 467,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo76 },  // Inst #467 = FLDGT_S
  { 466,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo75 },  // Inst #466 = FLDGT_D
  { 465,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo68 },  // Inst #465 = FFINT_S_W
  { 464,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo74 },  // Inst #464 = FFINT_S_L
  { 463,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo73 },  // Inst #463 = FFINT_D_W
  { 462,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo67 },  // Inst #462 = FFINT_D_L
  { 461,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo70 },  // Inst #461 = FDIV_S
  { 460,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo69 },  // Inst #460 = FDIV_D
  { 459,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo74 },  // Inst #459 = FCVT_S_D
  { 458,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo73 },  // Inst #458 = FCVT_D_S
  { 457,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo70 },  // Inst #457 = FCOPYSIGN_S
  { 456,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo69 },  // Inst #456 = FCOPYSIGN_D
  { 455,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo72 },  // Inst #455 = FCMP_SUN_S
  { 454,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo71 },  // Inst #454 = FCMP_SUN_D
  { 453,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo72 },  // Inst #453 = FCMP_SUNE_S
  { 452,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo71 },  // Inst #452 = FCMP_SUNE_D
  { 451,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo72 },  // Inst #451 = FCMP_SULT_S
  { 450,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo71 },  // Inst #450 = FCMP_SULT_D
  { 449,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo72 },  // Inst #449 = FCMP_SULE_S
  { 448,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo71 },  // Inst #448 = FCMP_SULE_D
  { 447,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo72 },  // Inst #447 = FCMP_SUEQ_S
  { 446,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo71 },  // Inst #446 = FCMP_SUEQ_D
  { 445,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo72 },  // Inst #445 = FCMP_SOR_S
  { 444,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo71 },  // Inst #444 = FCMP_SOR_D
  { 443,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo72 },  // Inst #443 = FCMP_SNE_S
  { 442,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo71 },  // Inst #442 = FCMP_SNE_D
  { 441,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo72 },  // Inst #441 = FCMP_SLT_S
  { 440,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo71 },  // Inst #440 = FCMP_SLT_D
  { 439,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo72 },  // Inst #439 = FCMP_SLE_S
  { 438,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo71 },  // Inst #438 = FCMP_SLE_D
  { 437,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo72 },  // Inst #437 = FCMP_SEQ_S
  { 436,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo71 },  // Inst #436 = FCMP_SEQ_D
  { 435,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo72 },  // Inst #435 = FCMP_SAF_S
  { 434,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo71 },  // Inst #434 = FCMP_SAF_D
  { 433,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo72 },  // Inst #433 = FCMP_CUN_S
  { 432,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo71 },  // Inst #432 = FCMP_CUN_D
  { 431,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo72 },  // Inst #431 = FCMP_CUNE_S
  { 430,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo71 },  // Inst #430 = FCMP_CUNE_D
  { 429,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo72 },  // Inst #429 = FCMP_CULT_S
  { 428,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo71 },  // Inst #428 = FCMP_CULT_D
  { 427,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo72 },  // Inst #427 = FCMP_CULE_S
  { 426,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo71 },  // Inst #426 = FCMP_CULE_D
  { 425,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo72 },  // Inst #425 = FCMP_CUEQ_S
  { 424,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo71 },  // Inst #424 = FCMP_CUEQ_D
  { 423,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo72 },  // Inst #423 = FCMP_COR_S
  { 422,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo71 },  // Inst #422 = FCMP_COR_D
  { 421,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo72 },  // Inst #421 = FCMP_CNE_S
  { 420,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo71 },  // Inst #420 = FCMP_CNE_D
  { 419,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo72 },  // Inst #419 = FCMP_CLT_S
  { 418,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo71 },  // Inst #418 = FCMP_CLT_D
  { 417,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo72 },  // Inst #417 = FCMP_CLE_S
  { 416,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo71 },  // Inst #416 = FCMP_CLE_D
  { 415,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo72 },  // Inst #415 = FCMP_CEQ_S
  { 414,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo71 },  // Inst #414 = FCMP_CEQ_D
  { 413,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo72 },  // Inst #413 = FCMP_CAF_S
  { 412,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo71 },  // Inst #412 = FCMP_CAF_D
  { 411,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo68 },  // Inst #411 = FCLASS_S
  { 410,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo67 },  // Inst #410 = FCLASS_D
  { 409,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo70 },  // Inst #409 = FADD_S
  { 408,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo69 },  // Inst #408 = FADD_D
  { 407,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo68 },  // Inst #407 = FABS_S
  { 406,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo67 },  // Inst #406 = FABS_D
  { 405,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo60 },  // Inst #405 = EXT_W_H
  { 404,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo60 },  // Inst #404 = EXT_W_B
  { 403,	0,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr },  // Inst #403 = ERTN
  { 402,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UsesCustomInserter), 0x0ULL, nullptr, OperandInfo46 },  // Inst #402 = DIV_WU
  { 401,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UsesCustomInserter), 0x0ULL, nullptr, OperandInfo46 },  // Inst #401 = DIV_W
  { 400,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UsesCustomInserter), 0x0ULL, nullptr, OperandInfo46 },  // Inst #400 = DIV_DU
  { 399,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UsesCustomInserter), 0x0ULL, nullptr, OperandInfo46 },  // Inst #399 = DIV_D
  { 398,	1,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo2 },  // Inst #398 = DBCL
  { 397,	1,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo2 },  // Inst #397 = DBAR
  { 396,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo60 },  // Inst #396 = CTZ_W
  { 395,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo60 },  // Inst #395 = CTZ_D
  { 394,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo60 },  // Inst #394 = CTO_W
  { 393,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo60 },  // Inst #393 = CTO_D
  { 392,	4,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo66 },  // Inst #392 = CSRXCHG
  { 391,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo65 },  // Inst #391 = CSRWR
  { 390,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo47 },  // Inst #390 = CSRRD
  { 389,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #389 = CRC_W_W_W
  { 388,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #388 = CRC_W_H_W
  { 387,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #387 = CRC_W_D_W
  { 386,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #386 = CRC_W_B_W
  { 385,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #385 = CRCC_W_W_W
  { 384,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #384 = CRCC_W_H_W
  { 383,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #383 = CRCC_W_D_W
  { 382,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #382 = CRCC_W_B_W
  { 381,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo60 },  // Inst #381 = CPUCFG
  { 380,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo60 },  // Inst #380 = CLZ_W
  { 379,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo60 },  // Inst #379 = CLZ_D
  { 378,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo60 },  // Inst #378 = CLO_W
  { 377,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo60 },  // Inst #377 = CLO_D
  { 376,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo64 },  // Inst #376 = CACOP
  { 375,	4,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo58 },  // Inst #375 = BYTEPICK_W
  { 374,	4,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo58 },  // Inst #374 = BYTEPICK_D
  { 373,	4,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo63 },  // Inst #373 = BSTRPICK_W
  { 372,	4,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo63 },  // Inst #372 = BSTRPICK_D
  { 371,	5,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo62 },  // Inst #371 = BSTRINS_W
  { 370,	5,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo62 },  // Inst #370 = BSTRINS_D
  { 369,	1,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo2 },  // Inst #369 = BREAK
  { 368,	2,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::Branch)|(1ULL<<MCID::Terminator), 0x0ULL, nullptr, OperandInfo47 },  // Inst #368 = BNEZ
  { 367,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::Branch)|(1ULL<<MCID::Terminator), 0x0ULL, nullptr, OperandInfo50 },  // Inst #367 = BNE
  { 366,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::Branch)|(1ULL<<MCID::Terminator), 0x0ULL, nullptr, OperandInfo50 },  // Inst #366 = BLTU
  { 365,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::Branch)|(1ULL<<MCID::Terminator), 0x0ULL, nullptr, OperandInfo50 },  // Inst #365 = BLT
  { 364,	1,	0,	4,	0,	0,	1,	0|(1ULL<<MCID::Call)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, ImplicitList3, OperandInfo2 },  // Inst #364 = BL
  { 363,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo60 },  // Inst #363 = BITREV_W
  { 362,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo60 },  // Inst #362 = BITREV_D
  { 361,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo60 },  // Inst #361 = BITREV_8B
  { 360,	2,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo60 },  // Inst #360 = BITREV_4B
  { 359,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::Branch)|(1ULL<<MCID::Terminator), 0x0ULL, nullptr, OperandInfo50 },  // Inst #359 = BGEU
  { 358,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::Branch)|(1ULL<<MCID::Terminator), 0x0ULL, nullptr, OperandInfo50 },  // Inst #358 = BGE
  { 357,	2,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::Branch)|(1ULL<<MCID::Terminator), 0x0ULL, nullptr, OperandInfo47 },  // Inst #357 = BEQZ
  { 356,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::Branch)|(1ULL<<MCID::Terminator), 0x0ULL, nullptr, OperandInfo50 },  // Inst #356 = BEQ
  { 355,	2,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::Branch)|(1ULL<<MCID::Terminator)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo61 },  // Inst #355 = BCNEZ
  { 354,	2,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::Branch)|(1ULL<<MCID::Terminator)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo61 },  // Inst #354 = BCEQZ
  { 353,	1,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::Branch)|(1ULL<<MCID::Terminator)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo2 },  // Inst #353 = B
  { 352,	2,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo60 },  // Inst #352 = ASRTLE_D
  { 351,	2,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo60 },  // Inst #351 = ASRTGT_D
  { 350,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #350 = ANDN
  { 349,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo50 },  // Inst #349 = ANDI
  { 348,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #348 = AND
  { 347,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo59 },  // Inst #347 = AMXOR_W
  { 346,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo59 },  // Inst #346 = AMXOR_DB_W
  { 345,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo59 },  // Inst #345 = AMXOR_DB_D
  { 344,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo59 },  // Inst #344 = AMXOR_D
  { 343,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo59 },  // Inst #343 = AMSWAP_W
  { 342,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo59 },  // Inst #342 = AMSWAP_DB_W
  { 341,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo59 },  // Inst #341 = AMSWAP_DB_D
  { 340,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo59 },  // Inst #340 = AMSWAP_D
  { 339,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo59 },  // Inst #339 = AMOR_W
  { 338,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo59 },  // Inst #338 = AMOR_DB_W
  { 337,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo59 },  // Inst #337 = AMOR_DB_D
  { 336,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo59 },  // Inst #336 = AMOR_D
  { 335,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo59 },  // Inst #335 = AMMIN_WU
  { 334,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo59 },  // Inst #334 = AMMIN_W
  { 333,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo59 },  // Inst #333 = AMMIN_DU
  { 332,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo59 },  // Inst #332 = AMMIN_DB_WU
  { 331,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo59 },  // Inst #331 = AMMIN_DB_W
  { 330,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo59 },  // Inst #330 = AMMIN_DB_DU
  { 329,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo59 },  // Inst #329 = AMMIN_DB_D
  { 328,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo59 },  // Inst #328 = AMMIN_D
  { 327,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo59 },  // Inst #327 = AMMAX_WU
  { 326,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo59 },  // Inst #326 = AMMAX_W
  { 325,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo59 },  // Inst #325 = AMMAX_DU
  { 324,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo59 },  // Inst #324 = AMMAX_DB_WU
  { 323,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo59 },  // Inst #323 = AMMAX_DB_W
  { 322,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo59 },  // Inst #322 = AMMAX_DB_DU
  { 321,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo59 },  // Inst #321 = AMMAX_DB_D
  { 320,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo59 },  // Inst #320 = AMMAX_D
  { 319,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo59 },  // Inst #319 = AMAND_W
  { 318,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo59 },  // Inst #318 = AMAND_DB_W
  { 317,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo59 },  // Inst #317 = AMAND_DB_D
  { 316,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo59 },  // Inst #316 = AMAND_D
  { 315,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo59 },  // Inst #315 = AMADD_W
  { 314,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo59 },  // Inst #314 = AMADD_DB_W
  { 313,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo59 },  // Inst #313 = AMADD_DB_D
  { 312,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo59 },  // Inst #312 = AMADD_D
  { 311,	4,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo58 },  // Inst #311 = ALSL_WU
  { 310,	4,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo58 },  // Inst #310 = ALSL_W
  { 309,	4,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo58 },  // Inst #309 = ALSL_D
  { 308,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #308 = ADD_W
  { 307,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo46 },  // Inst #307 = ADD_D
  { 306,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo50 },  // Inst #306 = ADDU16I_D
  { 305,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo50 },  // Inst #305 = ADDI_W
  { 304,	3,	1,	4,	0,	0,	0,	0, 0x0ULL, nullptr, OperandInfo50 },  // Inst #304 = ADDI_D
  { 303,	2,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::UsesCustomInserter)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo57 },  // Inst #303 = WRFCSR
  { 302,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::UsesCustomInserter), 0x0ULL, nullptr, OperandInfo47 },  // Inst #302 = RDFCSR
  { 301,	0,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr },  // Inst #301 = PseudoUNIMP
  { 300,	1,	0,	4,	0,	1,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Return)|(1ULL<<MCID::Barrier)|(1ULL<<MCID::Call)|(1ULL<<MCID::Terminator), 0x0ULL, ImplicitList2, OperandInfo56 },  // Inst #300 = PseudoTAILIndirect
  { 299,	1,	0,	4,	0,	1,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Return)|(1ULL<<MCID::Barrier)|(1ULL<<MCID::Call)|(1ULL<<MCID::Terminator), 0x0ULL, ImplicitList2, OperandInfo2 },  // Inst #299 = PseudoTAIL
  { 298,	3,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo51 },  // Inst #298 = PseudoST_CFR
  { 297,	0,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Return)|(1ULL<<MCID::Barrier)|(1ULL<<MCID::Terminator), 0x0ULL, nullptr, nullptr },  // Inst #297 = PseudoRET
  { 296,	7,	2,	44,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo55 },  // Inst #296 = PseudoMaskedCmpXchg32
  { 295,	6,	2,	36,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo52 },  // Inst #295 = PseudoMaskedAtomicSwap32
  { 294,	7,	3,	48,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo54 },  // Inst #294 = PseudoMaskedAtomicLoadUMin32
  { 293,	7,	3,	48,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo54 },  // Inst #293 = PseudoMaskedAtomicLoadUMax32
  { 292,	6,	2,	36,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo52 },  // Inst #292 = PseudoMaskedAtomicLoadSub32
  { 291,	6,	2,	36,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo52 },  // Inst #291 = PseudoMaskedAtomicLoadNand32
  { 290,	8,	3,	56,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo53 },  // Inst #290 = PseudoMaskedAtomicLoadMin32
  { 289,	8,	3,	56,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo53 },  // Inst #289 = PseudoMaskedAtomicLoadMax32
  { 288,	6,	2,	36,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo52 },  // Inst #288 = PseudoMaskedAtomicLoadAdd32
  { 287,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo47 },  // Inst #287 = PseudoLI_W
  { 286,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo47 },  // Inst #286 = PseudoLI_D
  { 285,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo51 },  // Inst #285 = PseudoLD_CFR
  { 284,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo47 },  // Inst #284 = PseudoLA_TLS_LE
  { 283,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo50 },  // Inst #283 = PseudoLA_TLS_LD_LARGE
  { 282,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo47 },  // Inst #282 = PseudoLA_TLS_LD
  { 281,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo50 },  // Inst #281 = PseudoLA_TLS_IE_LARGE
  { 280,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo47 },  // Inst #280 = PseudoLA_TLS_IE
  { 279,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo50 },  // Inst #279 = PseudoLA_TLS_GD_LARGE
  { 278,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo47 },  // Inst #278 = PseudoLA_TLS_GD
  { 277,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo50 },  // Inst #277 = PseudoLA_PCREL_LARGE
  { 276,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo47 },  // Inst #276 = PseudoLA_PCREL
  { 275,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo50 },  // Inst #275 = PseudoLA_GOT_LARGE
  { 274,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo47 },  // Inst #274 = PseudoLA_GOT
  { 273,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo50 },  // Inst #273 = PseudoLA_ABS_LARGE
  { 272,	2,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo47 },  // Inst #272 = PseudoLA_ABS
  { 271,	2,	0,	4,	0,	1,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Return)|(1ULL<<MCID::Barrier)|(1ULL<<MCID::Call)|(1ULL<<MCID::Terminator)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, ImplicitList2, OperandInfo47 },  // Inst #271 = PseudoJIRL_TAIL
  { 270,	2,	0,	4,	0,	0,	1,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Call)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, ImplicitList3, OperandInfo47 },  // Inst #270 = PseudoJIRL_CALL
  { 269,	5,	2,	36,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo49 },  // Inst #269 = PseudoCmpXchg64
  { 268,	5,	2,	36,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo49 },  // Inst #268 = PseudoCmpXchg32
  { 267,	1,	0,	4,	0,	0,	1,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Call), 0x0ULL, ImplicitList3, OperandInfo48 },  // Inst #267 = PseudoCALLIndirect
  { 266,	1,	0,	4,	0,	0,	1,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Call), 0x0ULL, ImplicitList3, OperandInfo2 },  // Inst #266 = PseudoCALL
  { 265,	1,	0,	4,	0,	1,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Return)|(1ULL<<MCID::Barrier)|(1ULL<<MCID::Call)|(1ULL<<MCID::Terminator)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, ImplicitList2, OperandInfo2 },  // Inst #265 = PseudoB_TAIL
  { 264,	2,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Branch)|(1ULL<<MCID::IndirectBranch)|(1ULL<<MCID::Barrier)|(1ULL<<MCID::Terminator), 0x0ULL, nullptr, OperandInfo47 },  // Inst #264 = PseudoBRIND
  { 263,	1,	0,	4,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Branch)|(1ULL<<MCID::Barrier)|(1ULL<<MCID::Terminator), 0x0ULL, nullptr, OperandInfo2 },  // Inst #263 = PseudoBR
  { 262,	5,	2,	24,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo45 },  // Inst #262 = PseudoAtomicSwap32
  { 261,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo46 },  // Inst #261 = PseudoAtomicStoreW
  { 260,	3,	1,	4,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo46 },  // Inst #260 = PseudoAtomicStoreD
  { 259,	5,	2,	24,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo45 },  // Inst #259 = PseudoAtomicLoadXor32
  { 258,	5,	2,	24,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo45 },  // Inst #258 = PseudoAtomicLoadSub32
  { 257,	5,	2,	24,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo45 },  // Inst #257 = PseudoAtomicLoadOr32
  { 256,	5,	2,	24,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo45 },  // Inst #256 = PseudoAtomicLoadNand64
  { 255,	5,	2,	24,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo45 },  // Inst #255 = PseudoAtomicLoadNand32
  { 254,	5,	2,	24,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo45 },  // Inst #254 = PseudoAtomicLoadAnd32
  { 253,	5,	2,	24,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo45 },  // Inst #253 = PseudoAtomicLoadAdd32
  { 252,	2,	0,	4,	0,	1,	1,	0|(1ULL<<MCID::Pseudo), 0x0ULL, ImplicitList1, OperandInfo10 },  // Inst #252 = ADJCALLSTACKUP
  { 251,	2,	0,	4,	0,	1,	1,	0|(1ULL<<MCID::Pseudo), 0x0ULL, ImplicitList1, OperandInfo10 },  // Inst #251 = ADJCALLSTACKDOWN
  { 250,	4,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo44 },  // Inst #250 = G_UBFX
  { 249,	4,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo44 },  // Inst #249 = G_SBFX
  { 248,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #248 = G_VECREDUCE_UMIN
  { 247,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #247 = G_VECREDUCE_UMAX
  { 246,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #246 = G_VECREDUCE_SMIN
  { 245,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #245 = G_VECREDUCE_SMAX
  { 244,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #244 = G_VECREDUCE_XOR
  { 243,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #243 = G_VECREDUCE_OR
  { 242,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #242 = G_VECREDUCE_AND
  { 241,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #241 = G_VECREDUCE_MUL
  { 240,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #240 = G_VECREDUCE_ADD
  { 239,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #239 = G_VECREDUCE_FMIN
  { 238,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #238 = G_VECREDUCE_FMAX
  { 237,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #237 = G_VECREDUCE_FMUL
  { 236,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #236 = G_VECREDUCE_FADD
  { 235,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo40 },  // Inst #235 = G_VECREDUCE_SEQ_FMUL
  { 234,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo40 },  // Inst #234 = G_VECREDUCE_SEQ_FADD
  { 233,	3,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo22 },  // Inst #233 = G_BZERO
  { 232,	4,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo43 },  // Inst #232 = G_MEMSET
  { 231,	4,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo43 },  // Inst #231 = G_MEMMOVE
  { 230,	3,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo40 },  // Inst #230 = G_MEMCPY_INLINE
  { 229,	4,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo43 },  // Inst #229 = G_MEMCPY
  { 228,	2,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::UnmodeledSideEffects)|(1ULL<<MCID::Convergent), 0x0ULL, nullptr, OperandInfo42 },  // Inst #228 = G_WRITE_REGISTER
  { 227,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::UnmodeledSideEffects)|(1ULL<<MCID::Convergent), 0x0ULL, nullptr, OperandInfo21 },  // Inst #227 = G_READ_REGISTER
  { 226,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayRaiseFPException)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo25 },  // Inst #226 = G_STRICT_FSQRT
  { 225,	4,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayRaiseFPException)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo19 },  // Inst #225 = G_STRICT_FMA
  { 224,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayRaiseFPException)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo18 },  // Inst #224 = G_STRICT_FREM
  { 223,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayRaiseFPException)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo18 },  // Inst #223 = G_STRICT_FDIV
  { 222,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayRaiseFPException)|(1ULL<<MCID::Commutable)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo18 },  // Inst #222 = G_STRICT_FMUL
  { 221,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayRaiseFPException)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo18 },  // Inst #221 = G_STRICT_FSUB
  { 220,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayRaiseFPException)|(1ULL<<MCID::Commutable)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo18 },  // Inst #220 = G_STRICT_FADD
  { 219,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo26 },  // Inst #219 = G_DYN_STACKALLOC
  { 218,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo21 },  // Inst #218 = G_JUMP_TABLE
  { 217,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo21 },  // Inst #217 = G_BLOCK_ADDR
  { 216,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #216 = G_ADDRSPACE_CAST
  { 215,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo25 },  // Inst #215 = G_FNEARBYINT
  { 214,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo25 },  // Inst #214 = G_FRINT
  { 213,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo25 },  // Inst #213 = G_FFLOOR
  { 212,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo25 },  // Inst #212 = G_FSQRT
  { 211,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo25 },  // Inst #211 = G_FSIN
  { 210,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo25 },  // Inst #210 = G_FCOS
  { 209,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo25 },  // Inst #209 = G_FCEIL
  { 208,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo25 },  // Inst #208 = G_BITREVERSE
  { 207,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo25 },  // Inst #207 = G_BSWAP
  { 206,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #206 = G_CTPOP
  { 205,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #205 = G_CTLZ_ZERO_UNDEF
  { 204,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #204 = G_CTLZ
  { 203,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #203 = G_CTTZ_ZERO_UNDEF
  { 202,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #202 = G_CTTZ
  { 201,	4,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo41 },  // Inst #201 = G_SHUFFLE_VECTOR
  { 200,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo40 },  // Inst #200 = G_EXTRACT_VECTOR_ELT
  { 199,	4,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo39 },  // Inst #199 = G_INSERT_VECTOR_ELT
  { 198,	3,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Branch)|(1ULL<<MCID::IndirectBranch)|(1ULL<<MCID::Barrier)|(1ULL<<MCID::Terminator), 0x0ULL, nullptr, OperandInfo38 },  // Inst #198 = G_BRJT
  { 197,	1,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Branch)|(1ULL<<MCID::Barrier)|(1ULL<<MCID::Terminator), 0x0ULL, nullptr, OperandInfo2 },  // Inst #197 = G_BR
  { 196,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #196 = G_LLROUND
  { 195,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #195 = G_LROUND
  { 194,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo25 },  // Inst #194 = G_ABS
  { 193,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, OperandInfo18 },  // Inst #193 = G_UMAX
  { 192,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, OperandInfo18 },  // Inst #192 = G_UMIN
  { 191,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, OperandInfo18 },  // Inst #191 = G_SMAX
  { 190,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, OperandInfo18 },  // Inst #190 = G_SMIN
  { 189,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo33 },  // Inst #189 = G_PTRMASK
  { 188,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo33 },  // Inst #188 = G_PTR_ADD
  { 187,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, OperandInfo18 },  // Inst #187 = G_FMAXIMUM
  { 186,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, OperandInfo18 },  // Inst #186 = G_FMINIMUM
  { 185,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, OperandInfo18 },  // Inst #185 = G_FMAXNUM_IEEE
  { 184,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, OperandInfo18 },  // Inst #184 = G_FMINNUM_IEEE
  { 183,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, OperandInfo18 },  // Inst #183 = G_FMAXNUM
  { 182,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, OperandInfo18 },  // Inst #182 = G_FMINNUM
  { 181,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo25 },  // Inst #181 = G_FCANONICALIZE
  { 180,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo32 },  // Inst #180 = G_IS_FPCLASS
  { 179,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo33 },  // Inst #179 = G_FCOPYSIGN
  { 178,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo25 },  // Inst #178 = G_FABS
  { 177,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #177 = G_UITOFP
  { 176,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #176 = G_SITOFP
  { 175,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #175 = G_FPTOUI
  { 174,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #174 = G_FPTOSI
  { 173,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #173 = G_FPTRUNC
  { 172,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #172 = G_FPEXT
  { 171,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo25 },  // Inst #171 = G_FNEG
  { 170,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo25 },  // Inst #170 = G_FLOG10
  { 169,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo25 },  // Inst #169 = G_FLOG2
  { 168,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo25 },  // Inst #168 = G_FLOG
  { 167,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo25 },  // Inst #167 = G_FEXP2
  { 166,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo25 },  // Inst #166 = G_FEXP
  { 165,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo33 },  // Inst #165 = G_FPOWI
  { 164,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo18 },  // Inst #164 = G_FPOW
  { 163,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo18 },  // Inst #163 = G_FREM
  { 162,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo18 },  // Inst #162 = G_FDIV
  { 161,	4,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo19 },  // Inst #161 = G_FMAD
  { 160,	4,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo19 },  // Inst #160 = G_FMA
  { 159,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, OperandInfo18 },  // Inst #159 = G_FMUL
  { 158,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo18 },  // Inst #158 = G_FSUB
  { 157,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, OperandInfo18 },  // Inst #157 = G_FADD
  { 156,	4,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo37 },  // Inst #156 = G_UDIVFIXSAT
  { 155,	4,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo37 },  // Inst #155 = G_SDIVFIXSAT
  { 154,	4,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo37 },  // Inst #154 = G_UDIVFIX
  { 153,	4,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo37 },  // Inst #153 = G_SDIVFIX
  { 152,	4,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, OperandInfo37 },  // Inst #152 = G_UMULFIXSAT
  { 151,	4,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, OperandInfo37 },  // Inst #151 = G_SMULFIXSAT
  { 150,	4,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, OperandInfo37 },  // Inst #150 = G_UMULFIX
  { 149,	4,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, OperandInfo37 },  // Inst #149 = G_SMULFIX
  { 148,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo33 },  // Inst #148 = G_SSHLSAT
  { 147,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo33 },  // Inst #147 = G_USHLSAT
  { 146,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo18 },  // Inst #146 = G_SSUBSAT
  { 145,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo18 },  // Inst #145 = G_USUBSAT
  { 144,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, OperandInfo18 },  // Inst #144 = G_SADDSAT
  { 143,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, OperandInfo18 },  // Inst #143 = G_UADDSAT
  { 142,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, OperandInfo18 },  // Inst #142 = G_SMULH
  { 141,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, OperandInfo18 },  // Inst #141 = G_UMULH
  { 140,	4,	2,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, OperandInfo30 },  // Inst #140 = G_SMULO
  { 139,	4,	2,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, OperandInfo30 },  // Inst #139 = G_UMULO
  { 138,	5,	2,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo36 },  // Inst #138 = G_SSUBE
  { 137,	4,	2,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo30 },  // Inst #137 = G_SSUBO
  { 136,	5,	2,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo36 },  // Inst #136 = G_SADDE
  { 135,	4,	2,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, OperandInfo30 },  // Inst #135 = G_SADDO
  { 134,	5,	2,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo36 },  // Inst #134 = G_USUBE
  { 133,	4,	2,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo30 },  // Inst #133 = G_USUBO
  { 132,	5,	2,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo36 },  // Inst #132 = G_UADDE
  { 131,	4,	2,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, OperandInfo30 },  // Inst #131 = G_UADDO
  { 130,	4,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo30 },  // Inst #130 = G_SELECT
  { 129,	4,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo35 },  // Inst #129 = G_FCMP
  { 128,	4,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo35 },  // Inst #128 = G_ICMP
  { 127,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo33 },  // Inst #127 = G_ROTL
  { 126,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo33 },  // Inst #126 = G_ROTR
  { 125,	4,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo34 },  // Inst #125 = G_FSHR
  { 124,	4,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo34 },  // Inst #124 = G_FSHL
  { 123,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo33 },  // Inst #123 = G_ASHR
  { 122,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo33 },  // Inst #122 = G_LSHR
  { 121,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo33 },  // Inst #121 = G_SHL
  { 120,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #120 = G_ZEXT
  { 119,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo17 },  // Inst #119 = G_SEXT_INREG
  { 118,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #118 = G_SEXT
  { 117,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo32 },  // Inst #117 = G_VAARG
  { 116,	1,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo20 },  // Inst #116 = G_VASTART
  { 115,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo21 },  // Inst #115 = G_FCONSTANT
  { 114,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo21 },  // Inst #114 = G_CONSTANT
  { 113,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #113 = G_TRUNC
  { 112,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #112 = G_ANYEXT
  { 111,	1,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::Variadic)|(1ULL<<MCID::UnmodeledSideEffects)|(1ULL<<MCID::Convergent), 0x0ULL, nullptr, OperandInfo2 },  // Inst #111 = G_INTRINSIC_W_SIDE_EFFECTS
  { 110,	1,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Variadic)|(1ULL<<MCID::Convergent), 0x0ULL, nullptr, OperandInfo2 },  // Inst #110 = G_INTRINSIC
  { 109,	0,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Terminator), 0x0ULL, nullptr, nullptr },  // Inst #109 = G_INVOKE_REGION_START
  { 108,	1,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Branch)|(1ULL<<MCID::IndirectBranch)|(1ULL<<MCID::Barrier)|(1ULL<<MCID::Terminator), 0x0ULL, nullptr, OperandInfo20 },  // Inst #108 = G_BRINDIRECT
  { 107,	2,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Branch)|(1ULL<<MCID::Terminator), 0x0ULL, nullptr, OperandInfo21 },  // Inst #107 = G_BRCOND
  { 106,	2,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo10 },  // Inst #106 = G_FENCE
  { 105,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo31 },  // Inst #105 = G_ATOMICRMW_UDEC_WRAP
  { 104,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo31 },  // Inst #104 = G_ATOMICRMW_UINC_WRAP
  { 103,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo31 },  // Inst #103 = G_ATOMICRMW_FMIN
  { 102,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo31 },  // Inst #102 = G_ATOMICRMW_FMAX
  { 101,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo31 },  // Inst #101 = G_ATOMICRMW_FSUB
  { 100,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo31 },  // Inst #100 = G_ATOMICRMW_FADD
  { 99,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo31 },  // Inst #99 = G_ATOMICRMW_UMIN
  { 98,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo31 },  // Inst #98 = G_ATOMICRMW_UMAX
  { 97,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo31 },  // Inst #97 = G_ATOMICRMW_MIN
  { 96,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo31 },  // Inst #96 = G_ATOMICRMW_MAX
  { 95,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo31 },  // Inst #95 = G_ATOMICRMW_XOR
  { 94,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo31 },  // Inst #94 = G_ATOMICRMW_OR
  { 93,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo31 },  // Inst #93 = G_ATOMICRMW_NAND
  { 92,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo31 },  // Inst #92 = G_ATOMICRMW_AND
  { 91,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo31 },  // Inst #91 = G_ATOMICRMW_SUB
  { 90,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo31 },  // Inst #90 = G_ATOMICRMW_ADD
  { 89,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo31 },  // Inst #89 = G_ATOMICRMW_XCHG
  { 88,	4,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo30 },  // Inst #88 = G_ATOMIC_CMPXCHG
  { 87,	5,	2,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo29 },  // Inst #87 = G_ATOMIC_CMPXCHG_WITH_SUCCESS
  { 86,	5,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo28 },  // Inst #86 = G_INDEXED_STORE
  { 85,	2,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayStore), 0x0ULL, nullptr, OperandInfo23 },  // Inst #85 = G_STORE
  { 84,	5,	2,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo27 },  // Inst #84 = G_INDEXED_ZEXTLOAD
  { 83,	5,	2,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo27 },  // Inst #83 = G_INDEXED_SEXTLOAD
  { 82,	5,	2,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo27 },  // Inst #82 = G_INDEXED_LOAD
  { 81,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo23 },  // Inst #81 = G_ZEXTLOAD
  { 80,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo23 },  // Inst #80 = G_SEXTLOAD
  { 79,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad), 0x0ULL, nullptr, OperandInfo23 },  // Inst #79 = G_LOAD
  { 78,	1,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo20 },  // Inst #78 = G_READCYCLECOUNTER
  { 77,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo25 },  // Inst #77 = G_INTRINSIC_ROUNDEVEN
  { 76,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #76 = G_INTRINSIC_LRINT
  { 75,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo25 },  // Inst #75 = G_INTRINSIC_ROUND
  { 74,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo25 },  // Inst #74 = G_INTRINSIC_TRUNC
  { 73,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo26 },  // Inst #73 = G_INTRINSIC_FPTRUNC_ROUND
  { 72,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo25 },  // Inst #72 = G_FREEZE
  { 71,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #71 = G_BITCAST
  { 70,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #70 = G_INTTOPTR
  { 69,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo23 },  // Inst #69 = G_PTRTOINT
  { 68,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Variadic), 0x0ULL, nullptr, OperandInfo23 },  // Inst #68 = G_CONCAT_VECTORS
  { 67,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Variadic), 0x0ULL, nullptr, OperandInfo23 },  // Inst #67 = G_BUILD_VECTOR_TRUNC
  { 66,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Variadic), 0x0ULL, nullptr, OperandInfo23 },  // Inst #66 = G_BUILD_VECTOR
  { 65,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Variadic), 0x0ULL, nullptr, OperandInfo23 },  // Inst #65 = G_MERGE_VALUES
  { 64,	4,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo24 },  // Inst #64 = G_INSERT
  { 63,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Variadic), 0x0ULL, nullptr, OperandInfo23 },  // Inst #63 = G_UNMERGE_VALUES
  { 62,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo22 },  // Inst #62 = G_EXTRACT
  { 61,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo21 },  // Inst #61 = G_GLOBAL_VALUE
  { 60,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo21 },  // Inst #60 = G_FRAME_INDEX
  { 59,	1,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Variadic), 0x0ULL, nullptr, OperandInfo20 },  // Inst #59 = G_PHI
  { 58,	1,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo20 },  // Inst #58 = G_IMPLICIT_DEF
  { 57,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, OperandInfo18 },  // Inst #57 = G_XOR
  { 56,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, OperandInfo18 },  // Inst #56 = G_OR
  { 55,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, OperandInfo18 },  // Inst #55 = G_AND
  { 54,	4,	2,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo19 },  // Inst #54 = G_UDIVREM
  { 53,	4,	2,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo19 },  // Inst #53 = G_SDIVREM
  { 52,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo18 },  // Inst #52 = G_UREM
  { 51,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo18 },  // Inst #51 = G_SREM
  { 50,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo18 },  // Inst #50 = G_UDIV
  { 49,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo18 },  // Inst #49 = G_SDIV
  { 48,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, OperandInfo18 },  // Inst #48 = G_MUL
  { 47,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo18 },  // Inst #47 = G_SUB
  { 46,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Commutable), 0x0ULL, nullptr, OperandInfo18 },  // Inst #46 = G_ADD
  { 45,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo17 },  // Inst #45 = G_ASSERT_ALIGN
  { 44,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo17 },  // Inst #44 = G_ASSERT_ZEXT
  { 43,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::PreISelOpcode)|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo17 },  // Inst #43 = G_ASSERT_SEXT
  { 42,	0,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Meta)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr },  // Inst #42 = MEMBARRIER
  { 41,	0,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Variadic)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr },  // Inst #41 = ICALL_BRANCH_FUNNEL
  { 40,	3,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Call)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UsesCustomInserter)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo16 },  // Inst #40 = PATCHABLE_TYPED_EVENT_CALL
  { 39,	2,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Call)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UsesCustomInserter)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo15 },  // Inst #39 = PATCHABLE_EVENT_CALL
  { 38,	0,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Return)|(1ULL<<MCID::UsesCustomInserter)|(1ULL<<MCID::Variadic)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr },  // Inst #38 = PATCHABLE_TAIL_CALL
  { 37,	0,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::UsesCustomInserter)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr },  // Inst #37 = PATCHABLE_FUNCTION_EXIT
  { 36,	0,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Return)|(1ULL<<MCID::Terminator)|(1ULL<<MCID::UsesCustomInserter)|(1ULL<<MCID::Variadic)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr },  // Inst #36 = PATCHABLE_RET
  { 35,	0,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::UsesCustomInserter)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr },  // Inst #35 = PATCHABLE_FUNCTION_ENTER
  { 34,	0,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UsesCustomInserter)|(1ULL<<MCID::Variadic)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr },  // Inst #34 = PATCHABLE_OP
  { 33,	1,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Branch)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::Terminator)|(1ULL<<MCID::UsesCustomInserter)|(1ULL<<MCID::Variadic)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo2 },  // Inst #33 = FAULTING_OP
  { 32,	2,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo14 },  // Inst #32 = LOCAL_ESCAPE
  { 31,	0,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Call)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UsesCustomInserter)|(1ULL<<MCID::Variadic)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr },  // Inst #31 = STATEPOINT
  { 30,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::UsesCustomInserter)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo13 },  // Inst #30 = PREALLOCATED_ARG
  { 29,	1,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::UsesCustomInserter)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo3 },  // Inst #29 = PREALLOCATED_SETUP
  { 28,	1,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::Rematerializable), 0x0ULL, nullptr, OperandInfo12 },  // Inst #28 = LOAD_STACK_GUARD
  { 27,	6,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Call)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::UsesCustomInserter)|(1ULL<<MCID::Variadic)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo11 },  // Inst #27 = PATCHPOINT
  { 26,	0,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Call)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::MayStore)|(1ULL<<MCID::UsesCustomInserter)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr },  // Inst #26 = FENTRY_CALL
  { 25,	2,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Call)|(1ULL<<MCID::MayLoad)|(1ULL<<MCID::UsesCustomInserter)|(1ULL<<MCID::Variadic)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo10 },  // Inst #25 = STACKMAP
  { 24,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Meta), 0x0ULL, nullptr, OperandInfo9 },  // Inst #24 = ARITH_FENCE
  { 23,	4,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Meta)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, OperandInfo8 },  // Inst #23 = PSEUDO_PROBE
  { 22,	1,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Meta), 0x0ULL, nullptr, OperandInfo3 },  // Inst #22 = LIFETIME_END
  { 21,	1,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Meta), 0x0ULL, nullptr, OperandInfo3 },  // Inst #21 = LIFETIME_START
  { 20,	0,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Variadic), 0x0ULL, nullptr, nullptr },  // Inst #20 = BUNDLE
  { 19,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::CheapAsAMove), 0x0ULL, nullptr, OperandInfo7 },  // Inst #19 = COPY
  { 18,	2,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Variadic)|(1ULL<<MCID::CheapAsAMove), 0x0ULL, nullptr, OperandInfo7 },  // Inst #18 = REG_SEQUENCE
  { 17,	1,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Meta), 0x0ULL, nullptr, OperandInfo2 },  // Inst #17 = DBG_LABEL
  { 16,	0,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Meta)|(1ULL<<MCID::Variadic), 0x0ULL, nullptr, nullptr },  // Inst #16 = DBG_PHI
  { 15,	0,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Meta)|(1ULL<<MCID::Variadic), 0x0ULL, nullptr, nullptr },  // Inst #15 = DBG_INSTR_REF
  { 14,	0,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Meta)|(1ULL<<MCID::Variadic), 0x0ULL, nullptr, nullptr },  // Inst #14 = DBG_VALUE_LIST
  { 13,	0,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Meta)|(1ULL<<MCID::Variadic), 0x0ULL, nullptr, nullptr },  // Inst #13 = DBG_VALUE
  { 12,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::CheapAsAMove), 0x0ULL, nullptr, OperandInfo4 },  // Inst #12 = COPY_TO_REGCLASS
  { 11,	4,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo6 },  // Inst #11 = SUBREG_TO_REG
  { 10,	1,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Meta)|(1ULL<<MCID::Rematerializable)|(1ULL<<MCID::CheapAsAMove), 0x0ULL, nullptr, OperandInfo2 },  // Inst #10 = IMPLICIT_DEF
  { 9,	4,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo5 },  // Inst #9 = INSERT_SUBREG
  { 8,	3,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo), 0x0ULL, nullptr, OperandInfo4 },  // Inst #8 = EXTRACT_SUBREG
  { 7,	0,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Meta)|(1ULL<<MCID::Variadic), 0x0ULL, nullptr, nullptr },  // Inst #7 = KILL
  { 6,	1,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::NotDuplicable), 0x0ULL, nullptr, OperandInfo3 },  // Inst #6 = ANNOTATION_LABEL
  { 5,	1,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Meta)|(1ULL<<MCID::NotDuplicable), 0x0ULL, nullptr, OperandInfo3 },  // Inst #5 = GC_LABEL
  { 4,	1,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Meta)|(1ULL<<MCID::NotDuplicable), 0x0ULL, nullptr, OperandInfo3 },  // Inst #4 = EH_LABEL
  { 3,	1,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Meta)|(1ULL<<MCID::NotDuplicable), 0x0ULL, nullptr, OperandInfo3 },  // Inst #3 = CFI_INSTRUCTION
  { 2,	0,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Variadic)|(1ULL<<MCID::UnmodeledSideEffects), 0x0ULL, nullptr, nullptr },  // Inst #2 = INLINEASM_BR
  { 1,	0,	0,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Variadic), 0x0ULL, nullptr, nullptr },  // Inst #1 = INLINEASM
  { 0,	1,	1,	0,	0,	0,	0,	0|(1ULL<<MCID::Pseudo)|(1ULL<<MCID::Variadic), 0x0ULL, nullptr, OperandInfo2 },  // Inst #0 = PHI
};


#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#endif
extern const char LoongArchInstrNameData[] = {
  /* 0 */ "G_FLOG10\0"
  /* 9 */ "PseudoMaskedAtomicLoadSub32\0"
  /* 37 */ "PseudoAtomicLoadSub32\0"
  /* 59 */ "PseudoMaskedAtomicLoadAdd32\0"
  /* 87 */ "PseudoAtomicLoadAdd32\0"
  /* 109 */ "PseudoAtomicLoadAnd32\0"
  /* 131 */ "PseudoMaskedAtomicLoadNand32\0"
  /* 160 */ "PseudoAtomicLoadNand32\0"
  /* 183 */ "PseudoMaskedCmpXchg32\0"
  /* 205 */ "PseudoCmpXchg32\0"
  /* 221 */ "PseudoMaskedAtomicLoadUMin32\0"
  /* 250 */ "PseudoMaskedAtomicLoadMin32\0"
  /* 278 */ "PseudoMaskedAtomicSwap32\0"
  /* 303 */ "PseudoAtomicSwap32\0"
  /* 322 */ "PseudoAtomicLoadOr32\0"
  /* 343 */ "PseudoAtomicLoadXor32\0"
  /* 365 */ "PseudoMaskedAtomicLoadUMax32\0"
  /* 394 */ "PseudoMaskedAtomicLoadMax32\0"
  /* 422 */ "G_FLOG2\0"
  /* 430 */ "G_FEXP2\0"
  /* 438 */ "MOVFR2GR_S_64\0"
  /* 452 */ "MOVGR2FR_W_64\0"
  /* 466 */ "PseudoAtomicLoadNand64\0"
  /* 489 */ "PseudoCmpXchg64\0"
  /* 505 */ "G_FMA\0"
  /* 511 */ "G_STRICT_FMA\0"
  /* 524 */ "BITREV_4B\0"
  /* 534 */ "BITREV_8B\0"
  /* 544 */ "INVTLB\0"
  /* 551 */ "G_FSUB\0"
  /* 558 */ "G_STRICT_FSUB\0"
  /* 572 */ "G_ATOMICRMW_FSUB\0"
  /* 589 */ "G_SUB\0"
  /* 595 */ "G_ATOMICRMW_SUB\0"
  /* 611 */ "LD_B\0"
  /* 616 */ "IOCSRRD_B\0"
  /* 626 */ "LDLE_B\0"
  /* 633 */ "STLE_B\0"
  /* 640 */ "IOCSRWR_B\0"
  /* 650 */ "LDGT_B\0"
  /* 657 */ "STGT_B\0"
  /* 664 */ "ST_B\0"
  /* 669 */ "EXT_W_B\0"
  /* 677 */ "LDX_B\0"
  /* 683 */ "STX_B\0"
  /* 689 */ "G_INTRINSIC\0"
  /* 701 */ "G_FPTRUNC\0"
  /* 711 */ "G_INTRINSIC_TRUNC\0"
  /* 729 */ "G_TRUNC\0"
  /* 737 */ "G_BUILD_VECTOR_TRUNC\0"
  /* 758 */ "G_DYN_STACKALLOC\0"
  /* 775 */ "G_FMAD\0"
  /* 782 */ "G_INDEXED_SEXTLOAD\0"
  /* 801 */ "G_SEXTLOAD\0"
  /* 812 */ "G_INDEXED_ZEXTLOAD\0"
  /* 831 */ "G_ZEXTLOAD\0"
  /* 842 */ "G_INDEXED_LOAD\0"
  /* 857 */ "G_LOAD\0"
  /* 864 */ "G_VECREDUCE_FADD\0"
  /* 881 */ "G_FADD\0"
  /* 888 */ "G_VECREDUCE_SEQ_FADD\0"
  /* 909 */ "G_STRICT_FADD\0"
  /* 923 */ "G_ATOMICRMW_FADD\0"
  /* 940 */ "G_VECREDUCE_ADD\0"
  /* 956 */ "G_ADD\0"
  /* 962 */ "G_PTR_ADD\0"
  /* 972 */ "G_ATOMICRMW_ADD\0"
  /* 988 */ "PseudoLA_TLS_GD\0"
  /* 1004 */ "PRELD\0"
  /* 1010 */ "PseudoLA_TLS_LD\0"
  /* 1026 */ "G_ATOMICRMW_NAND\0"
  /* 1043 */ "G_VECREDUCE_AND\0"
  /* 1059 */ "G_AND\0"
  /* 1065 */ "G_ATOMICRMW_AND\0"
  /* 1081 */ "LIFETIME_END\0"
  /* 1094 */ "PseudoBRIND\0"
  /* 1106 */ "G_BRCOND\0"
  /* 1115 */ "G_LLROUND\0"
  /* 1125 */ "G_LROUND\0"
  /* 1134 */ "G_INTRINSIC_ROUND\0"
  /* 1152 */ "G_INTRINSIC_FPTRUNC_ROUND\0"
  /* 1178 */ "LOAD_STACK_GUARD\0"
  /* 1195 */ "TLBRD\0"
  /* 1201 */ "CSRRD\0"
  /* 1207 */ "FMINA_D\0"
  /* 1215 */ "SRA_D\0"
  /* 1221 */ "FMAXA_D\0"
  /* 1229 */ "AMADD_DB_D\0"
  /* 1240 */ "AMAND_DB_D\0"
  /* 1251 */ "AMMIN_DB_D\0"
  /* 1262 */ "AMSWAP_DB_D\0"
  /* 1274 */ "AMOR_DB_D\0"
  /* 1284 */ "AMXOR_DB_D\0"
  /* 1295 */ "AMMAX_DB_D\0"
  /* 1306 */ "FSCALEB_D\0"
  /* 1316 */ "FLOGB_D\0"
  /* 1324 */ "FSUB_D\0"
  /* 1331 */ "FMSUB_D\0"
  /* 1339 */ "FNMSUB_D\0"
  /* 1348 */ "REVB_D\0"
  /* 1355 */ "SC_D\0"
  /* 1360 */ "FADD_D\0"
  /* 1367 */ "AMADD_D\0"
  /* 1375 */ "FMADD_D\0"
  /* 1383 */ "FNMADD_D\0"
  /* 1392 */ "FLD_D\0"
  /* 1398 */ "AMAND_D\0"
  /* 1406 */ "MOD_D\0"
  /* 1412 */ "IOCSRRD_D\0"
  /* 1422 */ "FCMP_CLE_D\0"
  /* 1433 */ "FLDLE_D\0"
  /* 1441 */ "FCMP_SLE_D\0"
  /* 1452 */ "ASRTLE_D\0"
  /* 1461 */ "FSTLE_D\0"
  /* 1469 */ "FCMP_CULE_D\0"
  /* 1481 */ "FCMP_SULE_D\0"
  /* 1493 */ "RDTIME_D\0"
  /* 1502 */ "FCMP_CNE_D\0"
  /* 1513 */ "FCMP_SNE_D\0"
  /* 1524 */ "FCMP_CUNE_D\0"
  /* 1536 */ "FCMP_SUNE_D\0"
  /* 1548 */ "FCMP_CAF_D\0"
  /* 1559 */ "FCMP_SAF_D\0"
  /* 1570 */ "FNEG_D\0"
  /* 1577 */ "MULH_D\0"
  /* 1584 */ "REVH_D\0"
  /* 1591 */ "LU32I_D\0"
  /* 1599 */ "LU52I_D\0"
  /* 1607 */ "ADDU16I_D\0"
  /* 1617 */ "SRAI_D\0"
  /* 1624 */ "ADDI_D\0"
  /* 1631 */ "SLLI_D\0"
  /* 1638 */ "SRLI_D\0"
  /* 1645 */ "PseudoLI_D\0"
  /* 1656 */ "ROTRI_D\0"
  /* 1664 */ "BYTEPICK_D\0"
  /* 1675 */ "BSTRPICK_D\0"
  /* 1686 */ "FSEL_D\0"
  /* 1693 */ "SLL_D\0"
  /* 1699 */ "SRL_D\0"
  /* 1705 */ "ALSL_D\0"
  /* 1712 */ "FMUL_D\0"
  /* 1719 */ "FTINTRNE_L_D\0"
  /* 1732 */ "FTINTRM_L_D\0"
  /* 1744 */ "FTINTRP_L_D\0"
  /* 1756 */ "FTINT_L_D\0"
  /* 1766 */ "FTINTRZ_L_D\0"
  /* 1778 */ "FCOPYSIGN_D\0"
  /* 1790 */ "FMIN_D\0"
  /* 1797 */ "AMMIN_D\0"
  /* 1805 */ "FCMP_CUN_D\0"
  /* 1816 */ "FCMP_SUN_D\0"
  /* 1827 */ "CLO_D\0"
  /* 1833 */ "CTO_D\0"
  /* 1839 */ "AMSWAP_D\0"
  /* 1848 */ "FRECIP_D\0"
  /* 1857 */ "FCMP_CEQ_D\0"
  /* 1868 */ "FCMP_SEQ_D\0"
  /* 1879 */ "FCMP_CUEQ_D\0"
  /* 1891 */ "FCMP_SUEQ_D\0"
  /* 1903 */ "MOVGR2FR_D\0"
  /* 1914 */ "MOVFR2GR_D\0"
  /* 1925 */ "FCMP_COR_D\0"
  /* 1936 */ "AMOR_D\0"
  /* 1943 */ "FCMP_SOR_D\0"
  /* 1954 */ "AMXOR_D\0"
  /* 1962 */ "ROTR_D\0"
  /* 1969 */ "LDPTR_D\0"
  /* 1977 */ "STPTR_D\0"
  /* 1985 */ "IOCSRWR_D\0"
  /* 1995 */ "FABS_D\0"
  /* 2002 */ "BSTRINS_D\0"
  /* 2012 */ "FCLASS_D\0"
  /* 2021 */ "FCVT_S_D\0"
  /* 2030 */ "FLDGT_D\0"
  /* 2038 */ "ASRTGT_D\0"
  /* 2047 */ "FSTGT_D\0"
  /* 2055 */ "FCMP_CLT_D\0"
  /* 2066 */ "FCMP_SLT_D\0"
  /* 2077 */ "FCMP_CULT_D\0"
  /* 2089 */ "FCMP_SULT_D\0"
  /* 2101 */ "FRINT_D\0"
  /* 2109 */ "FSQRT_D\0"
  /* 2117 */ "FRSQRT_D\0"
  /* 2126 */ "FST_D\0"
  /* 2132 */ "BITREV_D\0"
  /* 2141 */ "FDIV_D\0"
  /* 2148 */ "FMOV_D\0"
  /* 2155 */ "FTINTRNE_W_D\0"
  /* 2168 */ "FTINTRM_W_D\0"
  /* 2180 */ "FTINTRP_W_D\0"
  /* 2192 */ "FTINT_W_D\0"
  /* 2202 */ "FTINTRZ_W_D\0"
  /* 2214 */ "FMAX_D\0"
  /* 2221 */ "AMMAX_D\0"
  /* 2229 */ "FLDX_D\0"
  /* 2236 */ "FSTX_D\0"
  /* 2243 */ "CLZ_D\0"
  /* 2249 */ "CTZ_D\0"
  /* 2255 */ "PseudoAtomicStoreD\0"
  /* 2274 */ "PSEUDO_PROBE\0"
  /* 2287 */ "G_SSUBE\0"
  /* 2295 */ "G_USUBE\0"
  /* 2303 */ "G_FENCE\0"
  /* 2311 */ "ARITH_FENCE\0"
  /* 2323 */ "REG_SEQUENCE\0"
  /* 2336 */ "G_SADDE\0"
  /* 2344 */ "G_UADDE\0"
  /* 2352 */ "G_FMINNUM_IEEE\0"
  /* 2367 */ "G_FMAXNUM_IEEE\0"
  /* 2382 */ "BGE\0"
  /* 2386 */ "PseudoLA_TLS_GD_LARGE\0"
  /* 2408 */ "PseudoLA_TLS_LD_LARGE\0"
  /* 2430 */ "PseudoLA_TLS_IE_LARGE\0"
  /* 2452 */ "PseudoLA_PCREL_LARGE\0"
  /* 2473 */ "PseudoLA_ABS_LARGE\0"
  /* 2492 */ "PseudoLA_GOT_LARGE\0"
  /* 2511 */ "PseudoLA_TLS_IE\0"
  /* 2527 */ "G_JUMP_TABLE\0"
  /* 2540 */ "IDLE\0"
  /* 2545 */ "BUNDLE\0"
  /* 2552 */ "PseudoLA_TLS_LE\0"
  /* 2568 */ "BNE\0"
  /* 2572 */ "G_MEMCPY_INLINE\0"
  /* 2588 */ "LOCAL_ESCAPE\0"
  /* 2601 */ "G_INDEXED_STORE\0"
  /* 2617 */ "G_STORE\0"
  /* 2625 */ "G_BITREVERSE\0"
  /* 2638 */ "LDPTE\0"
  /* 2644 */ "DBG_VALUE\0"
  /* 2654 */ "G_GLOBAL_VALUE\0"
  /* 2669 */ "G_MEMMOVE\0"
  /* 2679 */ "G_FREEZE\0"
  /* 2688 */ "G_FCANONICALIZE\0"
  /* 2704 */ "MOVGR2CF\0"
  /* 2713 */ "G_CTLZ_ZERO_UNDEF\0"
  /* 2731 */ "G_CTTZ_ZERO_UNDEF\0"
  /* 2749 */ "G_IMPLICIT_DEF\0"
  /* 2764 */ "DBG_INSTR_REF\0"
  /* 2778 */ "G_FNEG\0"
  /* 2785 */ "EXTRACT_SUBREG\0"
  /* 2800 */ "INSERT_SUBREG\0"
  /* 2814 */ "G_SEXT_INREG\0"
  /* 2827 */ "SUBREG_TO_REG\0"
  /* 2841 */ "CPUCFG\0"
  /* 2848 */ "G_ATOMIC_CMPXCHG\0"
  /* 2865 */ "CSRXCHG\0"
  /* 2873 */ "G_ATOMICRMW_XCHG\0"
  /* 2890 */ "G_FLOG\0"
  /* 2897 */ "G_VAARG\0"
  /* 2905 */ "PREALLOCATED_ARG\0"
  /* 2922 */ "REVB_2H\0"
  /* 2930 */ "REVB_4H\0"
  /* 2938 */ "TLBSRCH\0"
  /* 2946 */ "G_SMULH\0"
  /* 2954 */ "G_UMULH\0"
  /* 2962 */ "TLBFLUSH\0"
  /* 2971 */ "LD_H\0"
  /* 2976 */ "IOCSRRD_H\0"
  /* 2986 */ "LDLE_H\0"
  /* 2993 */ "STLE_H\0"
  /* 3000 */ "IOCSRWR_H\0"
  /* 3010 */ "LDGT_H\0"
  /* 3017 */ "STGT_H\0"
  /* 3024 */ "ST_H\0"
  /* 3029 */ "EXT_W_H\0"
  /* 3037 */ "LDX_H\0"
  /* 3043 */ "STX_H\0"
  /* 3049 */ "PCALAU12I\0"
  /* 3059 */ "PCADDU12I\0"
  /* 3069 */ "PCADDU18I\0"
  /* 3079 */ "PCADDI\0"
  /* 3086 */ "ANDI\0"
  /* 3091 */ "DBG_PHI\0"
  /* 3099 */ "XORI\0"
  /* 3104 */ "G_FPTOSI\0"
  /* 3113 */ "SLTI\0"
  /* 3118 */ "G_FPTOUI\0"
  /* 3127 */ "SLTUI\0"
  /* 3133 */ "G_FPOWI\0"
  /* 3141 */ "BREAK\0"
  /* 3147 */ "G_PTRMASK\0"
  /* 3157 */ "BL\0"
  /* 3160 */ "DBCL\0"
  /* 3165 */ "GC_LABEL\0"
  /* 3174 */ "DBG_LABEL\0"
  /* 3184 */ "EH_LABEL\0"
  /* 3193 */ "ANNOTATION_LABEL\0"
  /* 3210 */ "ICALL_BRANCH_FUNNEL\0"
  /* 3230 */ "PseudoLA_PCREL\0"
  /* 3245 */ "G_FSHL\0"
  /* 3252 */ "G_SHL\0"
  /* 3258 */ "PseudoB_TAIL\0"
  /* 3271 */ "PseudoJIRL_TAIL\0"
  /* 3287 */ "PseudoTAIL\0"
  /* 3298 */ "G_FCEIL\0"
  /* 3306 */ "SYSCALL\0"
  /* 3314 */ "PATCHABLE_TAIL_CALL\0"
  /* 3334 */ "PseudoJIRL_CALL\0"
  /* 3350 */ "PATCHABLE_TYPED_EVENT_CALL\0"
  /* 3377 */ "PATCHABLE_EVENT_CALL\0"
  /* 3398 */ "FENTRY_CALL\0"
  /* 3410 */ "PseudoCALL\0"
  /* 3421 */ "TLBFILL\0"
  /* 3429 */ "KILL\0"
  /* 3434 */ "JIRL\0"
  /* 3439 */ "G_ROTL\0"
  /* 3446 */ "G_VECREDUCE_FMUL\0"
  /* 3463 */ "G_FMUL\0"
  /* 3470 */ "G_VECREDUCE_SEQ_FMUL\0"
  /* 3491 */ "G_STRICT_FMUL\0"
  /* 3505 */ "G_VECREDUCE_MUL\0"
  /* 3521 */ "G_MUL\0"
  /* 3527 */ "FFINT_D_L\0"
  /* 3537 */ "FFINT_S_L\0"
  /* 3547 */ "G_FREM\0"
  /* 3554 */ "G_STRICT_FREM\0"
  /* 3568 */ "G_SREM\0"
  /* 3575 */ "G_UREM\0"
  /* 3582 */ "G_SDIVREM\0"
  /* 3592 */ "G_UDIVREM\0"
  /* 3602 */ "INLINEASM\0"
  /* 3612 */ "G_FMINIMUM\0"
  /* 3623 */ "G_FMAXIMUM\0"
  /* 3634 */ "G_FMINNUM\0"
  /* 3644 */ "G_FMAXNUM\0"
  /* 3654 */ "ANDN\0"
  /* 3659 */ "G_INTRINSIC_ROUNDEVEN\0"
  /* 3681 */ "G_ASSERT_ALIGN\0"
  /* 3696 */ "G_FCOPYSIGN\0"
  /* 3708 */ "G_VECREDUCE_FMIN\0"
  /* 3725 */ "G_ATOMICRMW_FMIN\0"
  /* 3742 */ "G_VECREDUCE_SMIN\0"
  /* 3759 */ "G_SMIN\0"
  /* 3766 */ "G_VECREDUCE_UMIN\0"
  /* 3783 */ "G_UMIN\0"
  /* 3790 */ "G_ATOMICRMW_UMIN\0"
  /* 3807 */ "G_ATOMICRMW_MIN\0"
  /* 3823 */ "G_FSIN\0"
  /* 3830 */ "CFI_INSTRUCTION\0"
  /* 3846 */ "ORN\0"
  /* 3850 */ "ERTN\0"
  /* 3855 */ "ADJCALLSTACKDOWN\0"
  /* 3872 */ "G_SSUBO\0"
  /* 3880 */ "G_USUBO\0"
  /* 3888 */ "G_SADDO\0"
  /* 3896 */ "G_UADDO\0"
  /* 3904 */ "G_SMULO\0"
  /* 3912 */ "G_UMULO\0"
  /* 3920 */ "G_BZERO\0"
  /* 3928 */ "STACKMAP\0"
  /* 3937 */ "G_ATOMICRMW_UDEC_WRAP\0"
  /* 3959 */ "G_ATOMICRMW_UINC_WRAP\0"
  /* 3981 */ "G_BSWAP\0"
  /* 3989 */ "G_SITOFP\0"
  /* 3998 */ "G_UITOFP\0"
  /* 4007 */ "G_FCMP\0"
  /* 4014 */ "G_ICMP\0"
  /* 4021 */ "PseudoUNIMP\0"
  /* 4033 */ "CACOP\0"
  /* 4039 */ "G_CTPOP\0"
  /* 4047 */ "PATCHABLE_OP\0"
  /* 4060 */ "FAULTING_OP\0"
  /* 4072 */ "ADJCALLSTACKUP\0"
  /* 4087 */ "PREALLOCATED_SETUP\0"
  /* 4106 */ "G_FEXP\0"
  /* 4113 */ "BEQ\0"
  /* 4117 */ "DBAR\0"
  /* 4122 */ "IBAR\0"
  /* 4127 */ "G_BR\0"
  /* 4132 */ "INLINEASM_BR\0"
  /* 4145 */ "PseudoBR\0"
  /* 4154 */ "G_BLOCK_ADDR\0"
  /* 4167 */ "MEMBARRIER\0"
  /* 4178 */ "PATCHABLE_FUNCTION_ENTER\0"
  /* 4203 */ "G_READCYCLECOUNTER\0"
  /* 4222 */ "G_READ_REGISTER\0"
  /* 4238 */ "G_WRITE_REGISTER\0"
  /* 4255 */ "PseudoLD_CFR\0"
  /* 4268 */ "PseudoST_CFR\0"
  /* 4281 */ "MOVCF2GR\0"
  /* 4290 */ "MOVFCSR2GR\0"
  /* 4301 */ "G_ASHR\0"
  /* 4308 */ "G_FSHR\0"
  /* 4315 */ "G_LSHR\0"
  /* 4322 */ "LDDIR\0"
  /* 4328 */ "TLBCLR\0"
  /* 4335 */ "NOR\0"
  /* 4339 */ "G_FFLOOR\0"
  /* 4348 */ "G_BUILD_VECTOR\0"
  /* 4363 */ "G_SHUFFLE_VECTOR\0"
  /* 4380 */ "G_VECREDUCE_XOR\0"
  /* 4396 */ "G_XOR\0"
  /* 4402 */ "G_ATOMICRMW_XOR\0"
  /* 4418 */ "G_VECREDUCE_OR\0"
  /* 4433 */ "G_OR\0"
  /* 4438 */ "G_ATOMICRMW_OR\0"
  /* 4453 */ "MOVGR2FCSR\0"
  /* 4464 */ "RDFCSR\0"
  /* 4471 */ "WRFCSR\0"
  /* 4478 */ "G_ROTR\0"
  /* 4485 */ "G_INTTOPTR\0"
  /* 4496 */ "TLBWR\0"
  /* 4502 */ "CSRWR\0"
  /* 4508 */ "G_FABS\0"
  /* 4515 */ "PseudoLA_ABS\0"
  /* 4528 */ "G_ABS\0"
  /* 4534 */ "G_UNMERGE_VALUES\0"
  /* 4551 */ "G_MERGE_VALUES\0"
  /* 4566 */ "G_FCOS\0"
  /* 4573 */ "G_CONCAT_VECTORS\0"
  /* 4590 */ "COPY_TO_REGCLASS\0"
  /* 4607 */ "G_IS_FPCLASS\0"
  /* 4620 */ "G_ATOMIC_CMPXCHG_WITH_SUCCESS\0"
  /* 4650 */ "G_INTRINSIC_W_SIDE_EFFECTS\0"
  /* 4677 */ "FMINA_S\0"
  /* 4685 */ "FMAXA_S\0"
  /* 4693 */ "FSCALEB_S\0"
  /* 4703 */ "FLOGB_S\0"
  /* 4711 */ "FSUB_S\0"
  /* 4718 */ "FMSUB_S\0"
  /* 4726 */ "FNMSUB_S\0"
  /* 4735 */ "FADD_S\0"
  /* 4742 */ "FMADD_S\0"
  /* 4750 */ "FNMADD_S\0"
  /* 4759 */ "FLD_S\0"
  /* 4765 */ "FCVT_D_S\0"
  /* 4774 */ "FCMP_CLE_S\0"
  /* 4785 */ "FLDLE_S\0"
  /* 4793 */ "FCMP_SLE_S\0"
  /* 4804 */ "FSTLE_S\0"
  /* 4812 */ "FCMP_CULE_S\0"
  /* 4824 */ "FCMP_SULE_S\0"
  /* 4836 */ "FCMP_CNE_S\0"
  /* 4847 */ "FCMP_SNE_S\0"
  /* 4858 */ "FCMP_CUNE_S\0"
  /* 4870 */ "FCMP_SUNE_S\0"
  /* 4882 */ "FCMP_CAF_S\0"
  /* 4893 */ "FCMP_SAF_S\0"
  /* 4904 */ "MOVFR2CF_S\0"
  /* 4915 */ "FNEG_S\0"
  /* 4922 */ "FSEL_S\0"
  /* 4929 */ "FMUL_S\0"
  /* 4936 */ "FTINTRNE_L_S\0"
  /* 4949 */ "FTINTRM_L_S\0"
  /* 4961 */ "FTINTRP_L_S\0"
  /* 4973 */ "FTINT_L_S\0"
  /* 4983 */ "FTINTRZ_L_S\0"
  /* 4995 */ "FCOPYSIGN_S\0"
  /* 5007 */ "FMIN_S\0"
  /* 5014 */ "FCMP_CUN_S\0"
  /* 5025 */ "FCMP_SUN_S\0"
  /* 5036 */ "FRECIP_S\0"
  /* 5045 */ "FCMP_CEQ_S\0"
  /* 5056 */ "FCMP_SEQ_S\0"
  /* 5067 */ "FCMP_CUEQ_S\0"
  /* 5079 */ "FCMP_SUEQ_S\0"
  /* 5091 */ "MOVCF2FR_S\0"
  /* 5102 */ "MOVFRH2GR_S\0"
  /* 5114 */ "MOVFR2GR_S\0"
  /* 5125 */ "FCMP_COR_S\0"
  /* 5136 */ "FCMP_SOR_S\0"
  /* 5147 */ "FABS_S\0"
  /* 5154 */ "FCLASS_S\0"
  /* 5163 */ "FLDGT_S\0"
  /* 5171 */ "FSTGT_S\0"
  /* 5179 */ "FCMP_CLT_S\0"
  /* 5190 */ "FCMP_SLT_S\0"
  /* 5201 */ "FCMP_CULT_S\0"
  /* 5213 */ "FCMP_SULT_S\0"
  /* 5225 */ "FRINT_S\0"
  /* 5233 */ "FSQRT_S\0"
  /* 5241 */ "FRSQRT_S\0"
  /* 5250 */ "FST_S\0"
  /* 5256 */ "FDIV_S\0"
  /* 5263 */ "FMOV_S\0"
  /* 5270 */ "FTINTRNE_W_S\0"
  /* 5283 */ "FTINTRM_W_S\0"
  /* 5295 */ "FTINTRP_W_S\0"
  /* 5307 */ "FTINT_W_S\0"
  /* 5317 */ "FTINTRZ_W_S\0"
  /* 5329 */ "FMAX_S\0"
  /* 5336 */ "FLDX_S\0"
  /* 5343 */ "FSTX_S\0"
  /* 5350 */ "G_SSUBSAT\0"
  /* 5360 */ "G_USUBSAT\0"
  /* 5370 */ "G_SADDSAT\0"
  /* 5380 */ "G_UADDSAT\0"
  /* 5390 */ "G_SSHLSAT\0"
  /* 5400 */ "G_USHLSAT\0"
  /* 5410 */ "G_SMULFIXSAT\0"
  /* 5423 */ "G_UMULFIXSAT\0"
  /* 5436 */ "G_SDIVFIXSAT\0"
  /* 5449 */ "G_UDIVFIXSAT\0"
  /* 5462 */ "G_EXTRACT\0"
  /* 5472 */ "G_SELECT\0"
  /* 5481 */ "G_BRINDIRECT\0"
  /* 5494 */ "PATCHABLE_RET\0"
  /* 5508 */ "PseudoRET\0"
  /* 5518 */ "G_MEMSET\0"
  /* 5527 */ "PATCHABLE_FUNCTION_EXIT\0"
  /* 5551 */ "G_BRJT\0"
  /* 5558 */ "BLT\0"
  /* 5562 */ "G_EXTRACT_VECTOR_ELT\0"
  /* 5583 */ "G_INSERT_VECTOR_ELT\0"
  /* 5603 */ "SLT\0"
  /* 5607 */ "G_FCONSTANT\0"
  /* 5619 */ "G_CONSTANT\0"
  /* 5630 */ "STATEPOINT\0"
  /* 5641 */ "PATCHPOINT\0"
  /* 5652 */ "G_PTRTOINT\0"
  /* 5663 */ "G_FRINT\0"
  /* 5671 */ "G_INTRINSIC_LRINT\0"
  /* 5689 */ "G_FNEARBYINT\0"
  /* 5702 */ "PseudoLA_GOT\0"
  /* 5715 */ "G_VASTART\0"
  /* 5725 */ "LIFETIME_START\0"
  /* 5740 */ "G_INVOKE_REGION_START\0"
  /* 5762 */ "G_INSERT\0"
  /* 5771 */ "G_FSQRT\0"
  /* 5779 */ "G_STRICT_FSQRT\0"
  /* 5794 */ "G_BITCAST\0"
  /* 5804 */ "G_ADDRSPACE_CAST\0"
  /* 5821 */ "DBG_VALUE_LIST\0"
  /* 5836 */ "G_FPEXT\0"
  /* 5844 */ "G_SEXT\0"
  /* 5851 */ "G_ASSERT_SEXT\0"
  /* 5865 */ "G_ANYEXT\0"
  /* 5874 */ "G_ZEXT\0"
  /* 5881 */ "G_ASSERT_ZEXT\0"
  /* 5895 */ "LD_BU\0"
  /* 5901 */ "LDX_BU\0"
  /* 5908 */ "AMMIN_DB_DU\0"
  /* 5920 */ "AMMAX_DB_DU\0"
  /* 5932 */ "MOD_DU\0"
  /* 5939 */ "MULH_DU\0"
  /* 5947 */ "AMMIN_DU\0"
  /* 5956 */ "DIV_DU\0"
  /* 5963 */ "AMMAX_DU\0"
  /* 5972 */ "BGEU\0"
  /* 5977 */ "LD_HU\0"
  /* 5983 */ "LDX_HU\0"
  /* 5990 */ "BLTU\0"
  /* 5995 */ "SLTU\0"
  /* 6000 */ "AMMIN_DB_WU\0"
  /* 6012 */ "AMMAX_DB_WU\0"
  /* 6024 */ "LD_WU\0"
  /* 6030 */ "MOD_WU\0"
  /* 6037 */ "MULW_D_WU\0"
  /* 6047 */ "MULH_WU\0"
  /* 6055 */ "ALSL_WU\0"
  /* 6063 */ "AMMIN_WU\0"
  /* 6072 */ "DIV_WU\0"
  /* 6079 */ "AMMAX_WU\0"
  /* 6088 */ "LDX_WU\0"
  /* 6095 */ "G_FDIV\0"
  /* 6102 */ "G_STRICT_FDIV\0"
  /* 6116 */ "G_SDIV\0"
  /* 6123 */ "G_UDIV\0"
  /* 6130 */ "REVB_2W\0"
  /* 6138 */ "REVH_2W\0"
  /* 6146 */ "G_FPOW\0"
  /* 6153 */ "SRA_W\0"
  /* 6159 */ "AMADD_DB_W\0"
  /* 6170 */ "AMAND_DB_W\0"
  /* 6181 */ "AMMIN_DB_W\0"
  /* 6192 */ "AMSWAP_DB_W\0"
  /* 6204 */ "AMOR_DB_W\0"
  /* 6214 */ "AMXOR_DB_W\0"
  /* 6225 */ "AMMAX_DB_W\0"
  /* 6236 */ "SUB_W\0"
  /* 6242 */ "CRCC_W_B_W\0"
  /* 6253 */ "CRC_W_B_W\0"
  /* 6263 */ "SC_W\0"
  /* 6268 */ "AMADD_W\0"
  /* 6276 */ "LD_W\0"
  /* 6281 */ "AMAND_W\0"
  /* 6289 */ "MOD_W\0"
  /* 6295 */ "IOCSRRD_W\0"
  /* 6305 */ "FFINT_D_W\0"
  /* 6315 */ "MULW_D_W\0"
  /* 6324 */ "CRCC_W_D_W\0"
  /* 6335 */ "CRC_W_D_W\0"
  /* 6345 */ "LDLE_W\0"
  /* 6352 */ "STLE_W\0"
  /* 6359 */ "RDTIMEH_W\0"
  /* 6369 */ "MULH_W\0"
  /* 6376 */ "MOVGR2FRH_W\0"
  /* 6388 */ "CRCC_W_H_W\0"
  /* 6399 */ "CRC_W_H_W\0"
  /* 6409 */ "LU12I_W\0"
  /* 6417 */ "SRAI_W\0"
  /* 6424 */ "ADDI_W\0"
  /* 6431 */ "SLLI_W\0"
  /* 6438 */ "SRLI_W\0"
  /* 6445 */ "PseudoLI_W\0"
  /* 6456 */ "ROTRI_W\0"
  /* 6464 */ "BYTEPICK_W\0"
  /* 6475 */ "BSTRPICK_W\0"
  /* 6486 */ "RDTIMEL_W\0"
  /* 6496 */ "SLL_W\0"
  /* 6502 */ "SRL_W\0"
  /* 6508 */ "ALSL_W\0"
  /* 6515 */ "MUL_W\0"
  /* 6521 */ "AMMIN_W\0"
  /* 6529 */ "CLO_W\0"
  /* 6535 */ "CTO_W\0"
  /* 6541 */ "AMSWAP_W\0"
  /* 6550 */ "MOVGR2FR_W\0"
  /* 6561 */ "AMOR_W\0"
  /* 6568 */ "AMXOR_W\0"
  /* 6576 */ "ROTR_W\0"
  /* 6583 */ "LDPTR_W\0"
  /* 6591 */ "STPTR_W\0"
  /* 6599 */ "IOCSRWR_W\0"
  /* 6609 */ "BSTRINS_W\0"
  /* 6619 */ "FFINT_S_W\0"
  /* 6629 */ "LDGT_W\0"
  /* 6636 */ "STGT_W\0"
  /* 6643 */ "ST_W\0"
  /* 6648 */ "BITREV_W\0"
  /* 6657 */ "DIV_W\0"
  /* 6663 */ "CRCC_W_W_W\0"
  /* 6674 */ "CRC_W_W_W\0"
  /* 6684 */ "AMMAX_W\0"
  /* 6692 */ "LDX_W\0"
  /* 6698 */ "STX_W\0"
  /* 6704 */ "CLZ_W\0"
  /* 6710 */ "CTZ_W\0"
  /* 6716 */ "PseudoAtomicStoreW\0"
  /* 6735 */ "G_VECREDUCE_FMAX\0"
  /* 6752 */ "G_ATOMICRMW_FMAX\0"
  /* 6769 */ "G_VECREDUCE_SMAX\0"
  /* 6786 */ "G_SMAX\0"
  /* 6793 */ "G_VECREDUCE_UMAX\0"
  /* 6810 */ "G_UMAX\0"
  /* 6817 */ "G_ATOMICRMW_UMAX\0"
  /* 6834 */ "G_ATOMICRMW_MAX\0"
  /* 6850 */ "PRELDX\0"
  /* 6857 */ "G_FRAME_INDEX\0"
  /* 6871 */ "G_SBFX\0"
  /* 6878 */ "G_UBFX\0"
  /* 6885 */ "G_SMULFIX\0"
  /* 6895 */ "G_UMULFIX\0"
  /* 6905 */ "G_SDIVFIX\0"
  /* 6915 */ "G_UDIVFIX\0"
  /* 6925 */ "G_MEMCPY\0"
  /* 6934 */ "COPY\0"
  /* 6939 */ "BNEZ\0"
  /* 6944 */ "BCNEZ\0"
  /* 6950 */ "MASKNEZ\0"
  /* 6958 */ "G_CTLZ\0"
  /* 6965 */ "BEQZ\0"
  /* 6970 */ "BCEQZ\0"
  /* 6976 */ "MASKEQZ\0"
  /* 6984 */ "G_CTTZ\0"
  /* 6991 */ "PseudoTAILIndirect\0"
  /* 7010 */ "PseudoCALLIndirect\0"
};
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

extern const unsigned LoongArchInstrNameIndices[] = {
    3095U, 3602U, 4132U, 3830U, 3184U, 3165U, 3193U, 3429U, 
    2785U, 2800U, 2751U, 2827U, 4590U, 2644U, 5821U, 2764U, 
    3091U, 3174U, 2323U, 6934U, 2545U, 5725U, 1081U, 2274U, 
    2311U, 3928U, 3398U, 5641U, 1178U, 4087U, 2905U, 5630U, 
    2588U, 4060U, 4047U, 4178U, 5494U, 5527U, 3314U, 3377U, 
    3350U, 3210U, 4167U, 5851U, 5881U, 3681U, 956U, 589U, 
    3521U, 6116U, 6123U, 3568U, 3575U, 3582U, 3592U, 1059U, 
    4433U, 4396U, 2749U, 3093U, 6857U, 2654U, 5462U, 4534U, 
    5762U, 4551U, 4348U, 737U, 4573U, 5652U, 4485U, 5794U, 
    2679U, 1152U, 711U, 1134U, 5671U, 3659U, 4203U, 857U, 
    801U, 831U, 842U, 782U, 812U, 2617U, 2601U, 4620U, 
    2848U, 2873U, 972U, 595U, 1065U, 1026U, 4438U, 4402U, 
    6834U, 3807U, 6817U, 3790U, 923U, 572U, 6752U, 3725U, 
    3959U, 3937U, 2303U, 1106U, 5481U, 5740U, 689U, 4650U, 
    5865U, 729U, 5619U, 5607U, 5715U, 2897U, 5844U, 2814U, 
    5874U, 3252U, 4315U, 4301U, 3245U, 4308U, 4478U, 3439U, 
    4014U, 4007U, 5472U, 3896U, 2344U, 3880U, 2295U, 3888U, 
    2336U, 3872U, 2287U, 3912U, 3904U, 2954U, 2946U, 5380U, 
    5370U, 5360U, 5350U, 5400U, 5390U, 6885U, 6895U, 5410U, 
    5423U, 6905U, 6915U, 5436U, 5449U, 881U, 551U, 3463U, 
    505U, 775U, 6095U, 3547U, 6146U, 3133U, 4106U, 430U, 
    2890U, 422U, 0U, 2778U, 5836U, 701U, 3104U, 3118U, 
    3989U, 3998U, 4508U, 3696U, 4607U, 2688U, 3634U, 3644U, 
    2352U, 2367U, 3612U, 3623U, 962U, 3147U, 3759U, 6786U, 
    3783U, 6810U, 4528U, 1125U, 1115U, 4127U, 5551U, 5583U, 
    5562U, 4363U, 6984U, 2731U, 6958U, 2713U, 4039U, 3981U, 
    2625U, 3298U, 4566U, 3823U, 5771U, 4339U, 5663U, 5689U, 
    5804U, 4154U, 2527U, 758U, 909U, 558U, 3491U, 6102U, 
    3554U, 511U, 5779U, 4222U, 4238U, 6925U, 2572U, 2669U, 
    5518U, 3920U, 888U, 3470U, 864U, 3446U, 6735U, 3708U, 
    940U, 3505U, 1043U, 4418U, 4380U, 6769U, 3742U, 6793U, 
    3766U, 6871U, 6878U, 3855U, 4072U, 87U, 109U, 160U, 
    466U, 322U, 37U, 343U, 2255U, 6716U, 303U, 4145U, 
    1094U, 3258U, 3410U, 7010U, 205U, 489U, 3334U, 3271U, 
    4515U, 2473U, 5702U, 2492U, 3230U, 2452U, 988U, 2386U, 
    2511U, 2430U, 1010U, 2408U, 2552U, 4255U, 1645U, 6445U, 
    59U, 394U, 250U, 131U, 9U, 365U, 221U, 278U, 
    183U, 5508U, 4268U, 3287U, 6991U, 4021U, 4464U, 4471U, 
    1624U, 6424U, 1607U, 1361U, 6270U, 1705U, 6508U, 6055U, 
    1367U, 1229U, 6159U, 6268U, 1398U, 1240U, 6170U, 6281U, 
    2221U, 1295U, 5920U, 6225U, 6012U, 5963U, 6684U, 6079U, 
    1797U, 1251U, 5908U, 6181U, 6000U, 5947U, 6521U, 6063U, 
    1936U, 1274U, 6204U, 6561U, 1839U, 1262U, 6192U, 6541U, 
    1954U, 1284U, 6214U, 6568U, 1039U, 3086U, 3654U, 2038U, 
    1452U, 532U, 6970U, 6944U, 4113U, 6965U, 2382U, 5972U, 
    524U, 534U, 2132U, 6648U, 3157U, 5558U, 5990U, 2568U, 
    6939U, 3141U, 2002U, 6609U, 1675U, 6475U, 1664U, 6464U, 
    4033U, 1827U, 6529U, 2243U, 6704U, 2841U, 6242U, 6324U, 
    6388U, 6663U, 6253U, 6335U, 6399U, 6674U, 1201U, 4502U, 
    2865U, 1833U, 6535U, 2249U, 6710U, 4117U, 3160U, 2142U, 
    5956U, 6657U, 6072U, 3850U, 669U, 3029U, 1995U, 5147U, 
    1360U, 4735U, 2012U, 5154U, 1548U, 4882U, 1857U, 5045U, 
    1422U, 4774U, 2055U, 5179U, 1502U, 4836U, 1925U, 5125U, 
    1879U, 5067U, 1469U, 4812U, 2077U, 5201U, 1524U, 4858U, 
    1805U, 5014U, 1559U, 4893U, 1868U, 5056U, 1441U, 4793U, 
    2066U, 5190U, 1513U, 4847U, 1943U, 5136U, 1891U, 5079U, 
    1481U, 4824U, 2089U, 5213U, 1536U, 4870U, 1816U, 5025U, 
    1778U, 4995U, 4765U, 2021U, 2141U, 5256U, 3527U, 6305U, 
    3537U, 6619U, 2030U, 5163U, 1433U, 4785U, 2229U, 5336U, 
    1392U, 4759U, 1316U, 4703U, 1375U, 4742U, 1221U, 4685U, 
    2214U, 5329U, 1207U, 4677U, 1790U, 5007U, 2148U, 5263U, 
    1331U, 4718U, 1712U, 4929U, 1570U, 4915U, 1383U, 4750U, 
    1339U, 4726U, 1848U, 5036U, 2101U, 5225U, 2117U, 5241U, 
    1306U, 4693U, 1686U, 4922U, 2109U, 5233U, 2047U, 5171U, 
    1461U, 4804U, 2236U, 5343U, 2126U, 5250U, 1324U, 4711U, 
    1732U, 4949U, 2168U, 5283U, 1719U, 4936U, 2155U, 5270U, 
    1744U, 4961U, 2180U, 5295U, 1766U, 4983U, 2202U, 5317U, 
    1756U, 4973U, 2192U, 5307U, 4122U, 2540U, 544U, 616U, 
    1412U, 2976U, 6295U, 640U, 1985U, 3000U, 6599U, 3434U, 
    4322U, 650U, 2031U, 3010U, 6629U, 626U, 1434U, 2986U, 
    6345U, 2638U, 1969U, 6583U, 677U, 5901U, 2230U, 3037U, 
    5983U, 6692U, 6088U, 611U, 5895U, 1393U, 2971U, 5977U, 
    6276U, 6024U, 1694U, 6497U, 6409U, 1591U, 1599U, 6976U, 
    6950U, 1406U, 5932U, 6289U, 6030U, 5091U, 4281U, 4290U, 
    4904U, 1914U, 5114U, 438U, 5102U, 2704U, 4453U, 6376U, 
    1903U, 6550U, 452U, 1577U, 5939U, 6369U, 6047U, 6315U, 
    6037U, 1713U, 6515U, 4335U, 4336U, 3100U, 3846U, 3079U, 
    3059U, 3069U, 3049U, 1004U, 6850U, 6359U, 6486U, 1493U, 
    2922U, 6130U, 2930U, 1348U, 6138U, 1584U, 1656U, 6456U, 
    1962U, 6576U, 1355U, 6263U, 1631U, 6431U, 1693U, 6496U, 
    5603U, 3113U, 5995U, 3127U, 1617U, 6417U, 1215U, 6153U, 
    1638U, 6438U, 1699U, 6502U, 657U, 2048U, 3017U, 6636U, 
    633U, 1462U, 2993U, 6352U, 1977U, 6591U, 683U, 2237U, 
    3043U, 6698U, 664U, 2127U, 3024U, 6643U, 1325U, 6236U, 
    3306U, 4328U, 3421U, 2962U, 1195U, 2938U, 4496U, 4392U, 
    3099U, 
};

static inline void InitLoongArchMCInstrInfo(MCInstrInfo *II) {
  II->InitMCInstrInfo(LoongArchInsts, LoongArchInstrNameIndices, LoongArchInstrNameData, nullptr, nullptr, 681);
}

} // end namespace llvm
#endif // GET_INSTRINFO_MC_DESC

#ifdef GET_INSTRINFO_HEADER
#undef GET_INSTRINFO_HEADER
namespace llvm {
struct LoongArchGenInstrInfo : public TargetInstrInfo {
  explicit LoongArchGenInstrInfo(unsigned CFSetupOpcode = ~0u, unsigned CFDestroyOpcode = ~0u, unsigned CatchRetOpcode = ~0u, unsigned ReturnOpcode = ~0u);
  ~LoongArchGenInstrInfo() override = default;

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
extern const MCInstrDesc LoongArchInsts[];
extern const unsigned LoongArchInstrNameIndices[];
extern const char LoongArchInstrNameData[];
LoongArchGenInstrInfo::LoongArchGenInstrInfo(unsigned CFSetupOpcode, unsigned CFDestroyOpcode, unsigned CatchRetOpcode, unsigned ReturnOpcode)
  : TargetInstrInfo(CFSetupOpcode, CFDestroyOpcode, CatchRetOpcode, ReturnOpcode) {
  InitMCInstrInfo(LoongArchInsts, LoongArchInstrNameIndices, LoongArchInstrNameData, nullptr, nullptr, 681);
}
} // end namespace llvm
#endif // GET_INSTRINFO_CTOR_DTOR

#ifdef GET_INSTRINFO_OPERAND_ENUM
#undef GET_INSTRINFO_OPERAND_ENUM
namespace llvm {
namespace LoongArch {
namespace OpName {
enum {
  OPERAND_LAST
};
} // end namespace OpName
} // end namespace LoongArch
} // end namespace llvm
#endif //GET_INSTRINFO_OPERAND_ENUM

#ifdef GET_INSTRINFO_NAMED_OPS
#undef GET_INSTRINFO_NAMED_OPS
namespace llvm {
namespace LoongArch {
LLVM_READONLY
int16_t getNamedOperandIdx(uint16_t Opcode, uint16_t NamedIdx) {
  return -1;
}
} // end namespace LoongArch
} // end namespace llvm
#endif //GET_INSTRINFO_NAMED_OPS

#ifdef GET_INSTRINFO_OPERAND_TYPES_ENUM
#undef GET_INSTRINFO_OPERAND_TYPES_ENUM
namespace llvm {
namespace LoongArch {
namespace OpTypes {
enum OperandType {
  bare_symbol = 0,
  f32imm = 1,
  f64imm = 2,
  grlenimm = 3,
  i16imm = 4,
  i1imm = 5,
  i32imm = 6,
  i64imm = 7,
  i8imm = 8,
  imm32 = 9,
  ptype0 = 10,
  ptype1 = 11,
  ptype2 = 12,
  ptype3 = 13,
  ptype4 = 14,
  ptype5 = 15,
  simm12 = 16,
  simm12_addlike = 17,
  simm12_lu52id = 18,
  simm14_lsl2 = 19,
  simm16 = 20,
  simm16_lsl2 = 21,
  simm16_lsl2_br = 22,
  simm20 = 23,
  simm20_lu12iw = 24,
  simm20_lu32id = 25,
  simm20_pcalau12i = 26,
  simm21_lsl2 = 27,
  simm26_b = 28,
  simm26_symbol = 29,
  type0 = 30,
  type1 = 31,
  type2 = 32,
  type3 = 33,
  type4 = 34,
  type5 = 35,
  uimm12 = 36,
  uimm12_ori = 37,
  uimm14 = 38,
  uimm15 = 39,
  uimm2 = 40,
  uimm2_plus1 = 41,
  uimm3 = 42,
  uimm5 = 43,
  uimm6 = 44,
  uimm8 = 45,
  untyped_imm_0 = 46,
  GPRMemAtomic = 47,
  CFR = 48,
  FCSR = 49,
  FPR32 = 50,
  FPR64 = 51,
  GPR = 52,
  GPRT = 53,
  OPERAND_TYPE_LIST_END
};
} // end namespace OpTypes
} // end namespace LoongArch
} // end namespace llvm
#endif // GET_INSTRINFO_OPERAND_TYPES_ENUM

#ifdef GET_INSTRINFO_OPERAND_TYPE
#undef GET_INSTRINFO_OPERAND_TYPE
namespace llvm {
namespace LoongArch {
LLVM_READONLY
static int getOperandType(uint16_t Opcode, uint16_t OpIdx) {
  const uint16_t Offsets[] = {
    /* PHI */
    0,
    /* INLINEASM */
    1,
    /* INLINEASM_BR */
    1,
    /* CFI_INSTRUCTION */
    1,
    /* EH_LABEL */
    2,
    /* GC_LABEL */
    3,
    /* ANNOTATION_LABEL */
    4,
    /* KILL */
    5,
    /* EXTRACT_SUBREG */
    5,
    /* INSERT_SUBREG */
    8,
    /* IMPLICIT_DEF */
    12,
    /* SUBREG_TO_REG */
    13,
    /* COPY_TO_REGCLASS */
    17,
    /* DBG_VALUE */
    20,
    /* DBG_VALUE_LIST */
    20,
    /* DBG_INSTR_REF */
    20,
    /* DBG_PHI */
    20,
    /* DBG_LABEL */
    20,
    /* REG_SEQUENCE */
    21,
    /* COPY */
    23,
    /* BUNDLE */
    25,
    /* LIFETIME_START */
    25,
    /* LIFETIME_END */
    26,
    /* PSEUDO_PROBE */
    27,
    /* ARITH_FENCE */
    31,
    /* STACKMAP */
    33,
    /* FENTRY_CALL */
    35,
    /* PATCHPOINT */
    35,
    /* LOAD_STACK_GUARD */
    41,
    /* PREALLOCATED_SETUP */
    42,
    /* PREALLOCATED_ARG */
    43,
    /* STATEPOINT */
    46,
    /* LOCAL_ESCAPE */
    46,
    /* FAULTING_OP */
    48,
    /* PATCHABLE_OP */
    49,
    /* PATCHABLE_FUNCTION_ENTER */
    49,
    /* PATCHABLE_RET */
    49,
    /* PATCHABLE_FUNCTION_EXIT */
    49,
    /* PATCHABLE_TAIL_CALL */
    49,
    /* PATCHABLE_EVENT_CALL */
    49,
    /* PATCHABLE_TYPED_EVENT_CALL */
    51,
    /* ICALL_BRANCH_FUNNEL */
    54,
    /* MEMBARRIER */
    54,
    /* G_ASSERT_SEXT */
    54,
    /* G_ASSERT_ZEXT */
    57,
    /* G_ASSERT_ALIGN */
    60,
    /* G_ADD */
    63,
    /* G_SUB */
    66,
    /* G_MUL */
    69,
    /* G_SDIV */
    72,
    /* G_UDIV */
    75,
    /* G_SREM */
    78,
    /* G_UREM */
    81,
    /* G_SDIVREM */
    84,
    /* G_UDIVREM */
    88,
    /* G_AND */
    92,
    /* G_OR */
    95,
    /* G_XOR */
    98,
    /* G_IMPLICIT_DEF */
    101,
    /* G_PHI */
    102,
    /* G_FRAME_INDEX */
    103,
    /* G_GLOBAL_VALUE */
    105,
    /* G_EXTRACT */
    107,
    /* G_UNMERGE_VALUES */
    110,
    /* G_INSERT */
    112,
    /* G_MERGE_VALUES */
    116,
    /* G_BUILD_VECTOR */
    118,
    /* G_BUILD_VECTOR_TRUNC */
    120,
    /* G_CONCAT_VECTORS */
    122,
    /* G_PTRTOINT */
    124,
    /* G_INTTOPTR */
    126,
    /* G_BITCAST */
    128,
    /* G_FREEZE */
    130,
    /* G_INTRINSIC_FPTRUNC_ROUND */
    132,
    /* G_INTRINSIC_TRUNC */
    135,
    /* G_INTRINSIC_ROUND */
    137,
    /* G_INTRINSIC_LRINT */
    139,
    /* G_INTRINSIC_ROUNDEVEN */
    141,
    /* G_READCYCLECOUNTER */
    143,
    /* G_LOAD */
    144,
    /* G_SEXTLOAD */
    146,
    /* G_ZEXTLOAD */
    148,
    /* G_INDEXED_LOAD */
    150,
    /* G_INDEXED_SEXTLOAD */
    155,
    /* G_INDEXED_ZEXTLOAD */
    160,
    /* G_STORE */
    165,
    /* G_INDEXED_STORE */
    167,
    /* G_ATOMIC_CMPXCHG_WITH_SUCCESS */
    172,
    /* G_ATOMIC_CMPXCHG */
    177,
    /* G_ATOMICRMW_XCHG */
    181,
    /* G_ATOMICRMW_ADD */
    184,
    /* G_ATOMICRMW_SUB */
    187,
    /* G_ATOMICRMW_AND */
    190,
    /* G_ATOMICRMW_NAND */
    193,
    /* G_ATOMICRMW_OR */
    196,
    /* G_ATOMICRMW_XOR */
    199,
    /* G_ATOMICRMW_MAX */
    202,
    /* G_ATOMICRMW_MIN */
    205,
    /* G_ATOMICRMW_UMAX */
    208,
    /* G_ATOMICRMW_UMIN */
    211,
    /* G_ATOMICRMW_FADD */
    214,
    /* G_ATOMICRMW_FSUB */
    217,
    /* G_ATOMICRMW_FMAX */
    220,
    /* G_ATOMICRMW_FMIN */
    223,
    /* G_ATOMICRMW_UINC_WRAP */
    226,
    /* G_ATOMICRMW_UDEC_WRAP */
    229,
    /* G_FENCE */
    232,
    /* G_BRCOND */
    234,
    /* G_BRINDIRECT */
    236,
    /* G_INVOKE_REGION_START */
    237,
    /* G_INTRINSIC */
    237,
    /* G_INTRINSIC_W_SIDE_EFFECTS */
    238,
    /* G_ANYEXT */
    239,
    /* G_TRUNC */
    241,
    /* G_CONSTANT */
    243,
    /* G_FCONSTANT */
    245,
    /* G_VASTART */
    247,
    /* G_VAARG */
    248,
    /* G_SEXT */
    251,
    /* G_SEXT_INREG */
    253,
    /* G_ZEXT */
    256,
    /* G_SHL */
    258,
    /* G_LSHR */
    261,
    /* G_ASHR */
    264,
    /* G_FSHL */
    267,
    /* G_FSHR */
    271,
    /* G_ROTR */
    275,
    /* G_ROTL */
    278,
    /* G_ICMP */
    281,
    /* G_FCMP */
    285,
    /* G_SELECT */
    289,
    /* G_UADDO */
    293,
    /* G_UADDE */
    297,
    /* G_USUBO */
    302,
    /* G_USUBE */
    306,
    /* G_SADDO */
    311,
    /* G_SADDE */
    315,
    /* G_SSUBO */
    320,
    /* G_SSUBE */
    324,
    /* G_UMULO */
    329,
    /* G_SMULO */
    333,
    /* G_UMULH */
    337,
    /* G_SMULH */
    340,
    /* G_UADDSAT */
    343,
    /* G_SADDSAT */
    346,
    /* G_USUBSAT */
    349,
    /* G_SSUBSAT */
    352,
    /* G_USHLSAT */
    355,
    /* G_SSHLSAT */
    358,
    /* G_SMULFIX */
    361,
    /* G_UMULFIX */
    365,
    /* G_SMULFIXSAT */
    369,
    /* G_UMULFIXSAT */
    373,
    /* G_SDIVFIX */
    377,
    /* G_UDIVFIX */
    381,
    /* G_SDIVFIXSAT */
    385,
    /* G_UDIVFIXSAT */
    389,
    /* G_FADD */
    393,
    /* G_FSUB */
    396,
    /* G_FMUL */
    399,
    /* G_FMA */
    402,
    /* G_FMAD */
    406,
    /* G_FDIV */
    410,
    /* G_FREM */
    413,
    /* G_FPOW */
    416,
    /* G_FPOWI */
    419,
    /* G_FEXP */
    422,
    /* G_FEXP2 */
    424,
    /* G_FLOG */
    426,
    /* G_FLOG2 */
    428,
    /* G_FLOG10 */
    430,
    /* G_FNEG */
    432,
    /* G_FPEXT */
    434,
    /* G_FPTRUNC */
    436,
    /* G_FPTOSI */
    438,
    /* G_FPTOUI */
    440,
    /* G_SITOFP */
    442,
    /* G_UITOFP */
    444,
    /* G_FABS */
    446,
    /* G_FCOPYSIGN */
    448,
    /* G_IS_FPCLASS */
    451,
    /* G_FCANONICALIZE */
    454,
    /* G_FMINNUM */
    456,
    /* G_FMAXNUM */
    459,
    /* G_FMINNUM_IEEE */
    462,
    /* G_FMAXNUM_IEEE */
    465,
    /* G_FMINIMUM */
    468,
    /* G_FMAXIMUM */
    471,
    /* G_PTR_ADD */
    474,
    /* G_PTRMASK */
    477,
    /* G_SMIN */
    480,
    /* G_SMAX */
    483,
    /* G_UMIN */
    486,
    /* G_UMAX */
    489,
    /* G_ABS */
    492,
    /* G_LROUND */
    494,
    /* G_LLROUND */
    496,
    /* G_BR */
    498,
    /* G_BRJT */
    499,
    /* G_INSERT_VECTOR_ELT */
    502,
    /* G_EXTRACT_VECTOR_ELT */
    506,
    /* G_SHUFFLE_VECTOR */
    509,
    /* G_CTTZ */
    513,
    /* G_CTTZ_ZERO_UNDEF */
    515,
    /* G_CTLZ */
    517,
    /* G_CTLZ_ZERO_UNDEF */
    519,
    /* G_CTPOP */
    521,
    /* G_BSWAP */
    523,
    /* G_BITREVERSE */
    525,
    /* G_FCEIL */
    527,
    /* G_FCOS */
    529,
    /* G_FSIN */
    531,
    /* G_FSQRT */
    533,
    /* G_FFLOOR */
    535,
    /* G_FRINT */
    537,
    /* G_FNEARBYINT */
    539,
    /* G_ADDRSPACE_CAST */
    541,
    /* G_BLOCK_ADDR */
    543,
    /* G_JUMP_TABLE */
    545,
    /* G_DYN_STACKALLOC */
    547,
    /* G_STRICT_FADD */
    550,
    /* G_STRICT_FSUB */
    553,
    /* G_STRICT_FMUL */
    556,
    /* G_STRICT_FDIV */
    559,
    /* G_STRICT_FREM */
    562,
    /* G_STRICT_FMA */
    565,
    /* G_STRICT_FSQRT */
    569,
    /* G_READ_REGISTER */
    571,
    /* G_WRITE_REGISTER */
    573,
    /* G_MEMCPY */
    575,
    /* G_MEMCPY_INLINE */
    579,
    /* G_MEMMOVE */
    582,
    /* G_MEMSET */
    586,
    /* G_BZERO */
    590,
    /* G_VECREDUCE_SEQ_FADD */
    593,
    /* G_VECREDUCE_SEQ_FMUL */
    596,
    /* G_VECREDUCE_FADD */
    599,
    /* G_VECREDUCE_FMUL */
    601,
    /* G_VECREDUCE_FMAX */
    603,
    /* G_VECREDUCE_FMIN */
    605,
    /* G_VECREDUCE_ADD */
    607,
    /* G_VECREDUCE_MUL */
    609,
    /* G_VECREDUCE_AND */
    611,
    /* G_VECREDUCE_OR */
    613,
    /* G_VECREDUCE_XOR */
    615,
    /* G_VECREDUCE_SMAX */
    617,
    /* G_VECREDUCE_SMIN */
    619,
    /* G_VECREDUCE_UMAX */
    621,
    /* G_VECREDUCE_UMIN */
    623,
    /* G_SBFX */
    625,
    /* G_UBFX */
    629,
    /* ADJCALLSTACKDOWN */
    633,
    /* ADJCALLSTACKUP */
    635,
    /* PseudoAtomicLoadAdd32 */
    637,
    /* PseudoAtomicLoadAnd32 */
    642,
    /* PseudoAtomicLoadNand32 */
    647,
    /* PseudoAtomicLoadNand64 */
    652,
    /* PseudoAtomicLoadOr32 */
    657,
    /* PseudoAtomicLoadSub32 */
    662,
    /* PseudoAtomicLoadXor32 */
    667,
    /* PseudoAtomicStoreD */
    672,
    /* PseudoAtomicStoreW */
    675,
    /* PseudoAtomicSwap32 */
    678,
    /* PseudoBR */
    683,
    /* PseudoBRIND */
    684,
    /* PseudoB_TAIL */
    686,
    /* PseudoCALL */
    687,
    /* PseudoCALLIndirect */
    688,
    /* PseudoCmpXchg32 */
    689,
    /* PseudoCmpXchg64 */
    694,
    /* PseudoJIRL_CALL */
    699,
    /* PseudoJIRL_TAIL */
    701,
    /* PseudoLA_ABS */
    703,
    /* PseudoLA_ABS_LARGE */
    705,
    /* PseudoLA_GOT */
    708,
    /* PseudoLA_GOT_LARGE */
    710,
    /* PseudoLA_PCREL */
    713,
    /* PseudoLA_PCREL_LARGE */
    715,
    /* PseudoLA_TLS_GD */
    718,
    /* PseudoLA_TLS_GD_LARGE */
    720,
    /* PseudoLA_TLS_IE */
    723,
    /* PseudoLA_TLS_IE_LARGE */
    725,
    /* PseudoLA_TLS_LD */
    728,
    /* PseudoLA_TLS_LD_LARGE */
    730,
    /* PseudoLA_TLS_LE */
    733,
    /* PseudoLD_CFR */
    735,
    /* PseudoLI_D */
    738,
    /* PseudoLI_W */
    740,
    /* PseudoMaskedAtomicLoadAdd32 */
    742,
    /* PseudoMaskedAtomicLoadMax32 */
    748,
    /* PseudoMaskedAtomicLoadMin32 */
    756,
    /* PseudoMaskedAtomicLoadNand32 */
    764,
    /* PseudoMaskedAtomicLoadSub32 */
    770,
    /* PseudoMaskedAtomicLoadUMax32 */
    776,
    /* PseudoMaskedAtomicLoadUMin32 */
    783,
    /* PseudoMaskedAtomicSwap32 */
    790,
    /* PseudoMaskedCmpXchg32 */
    796,
    /* PseudoRET */
    803,
    /* PseudoST_CFR */
    803,
    /* PseudoTAIL */
    806,
    /* PseudoTAILIndirect */
    807,
    /* PseudoUNIMP */
    808,
    /* RDFCSR */
    808,
    /* WRFCSR */
    810,
    /* ADDI_D */
    812,
    /* ADDI_W */
    815,
    /* ADDU16I_D */
    818,
    /* ADD_D */
    821,
    /* ADD_W */
    824,
    /* ALSL_D */
    827,
    /* ALSL_W */
    831,
    /* ALSL_WU */
    835,
    /* AMADD_D */
    839,
    /* AMADD_DB_D */
    842,
    /* AMADD_DB_W */
    845,
    /* AMADD_W */
    848,
    /* AMAND_D */
    851,
    /* AMAND_DB_D */
    854,
    /* AMAND_DB_W */
    857,
    /* AMAND_W */
    860,
    /* AMMAX_D */
    863,
    /* AMMAX_DB_D */
    866,
    /* AMMAX_DB_DU */
    869,
    /* AMMAX_DB_W */
    872,
    /* AMMAX_DB_WU */
    875,
    /* AMMAX_DU */
    878,
    /* AMMAX_W */
    881,
    /* AMMAX_WU */
    884,
    /* AMMIN_D */
    887,
    /* AMMIN_DB_D */
    890,
    /* AMMIN_DB_DU */
    893,
    /* AMMIN_DB_W */
    896,
    /* AMMIN_DB_WU */
    899,
    /* AMMIN_DU */
    902,
    /* AMMIN_W */
    905,
    /* AMMIN_WU */
    908,
    /* AMOR_D */
    911,
    /* AMOR_DB_D */
    914,
    /* AMOR_DB_W */
    917,
    /* AMOR_W */
    920,
    /* AMSWAP_D */
    923,
    /* AMSWAP_DB_D */
    926,
    /* AMSWAP_DB_W */
    929,
    /* AMSWAP_W */
    932,
    /* AMXOR_D */
    935,
    /* AMXOR_DB_D */
    938,
    /* AMXOR_DB_W */
    941,
    /* AMXOR_W */
    944,
    /* AND */
    947,
    /* ANDI */
    950,
    /* ANDN */
    953,
    /* ASRTGT_D */
    956,
    /* ASRTLE_D */
    958,
    /* B */
    960,
    /* BCEQZ */
    961,
    /* BCNEZ */
    963,
    /* BEQ */
    965,
    /* BEQZ */
    968,
    /* BGE */
    970,
    /* BGEU */
    973,
    /* BITREV_4B */
    976,
    /* BITREV_8B */
    978,
    /* BITREV_D */
    980,
    /* BITREV_W */
    982,
    /* BL */
    984,
    /* BLT */
    985,
    /* BLTU */
    988,
    /* BNE */
    991,
    /* BNEZ */
    994,
    /* BREAK */
    996,
    /* BSTRINS_D */
    997,
    /* BSTRINS_W */
    1002,
    /* BSTRPICK_D */
    1007,
    /* BSTRPICK_W */
    1011,
    /* BYTEPICK_D */
    1015,
    /* BYTEPICK_W */
    1019,
    /* CACOP */
    1023,
    /* CLO_D */
    1026,
    /* CLO_W */
    1028,
    /* CLZ_D */
    1030,
    /* CLZ_W */
    1032,
    /* CPUCFG */
    1034,
    /* CRCC_W_B_W */
    1036,
    /* CRCC_W_D_W */
    1039,
    /* CRCC_W_H_W */
    1042,
    /* CRCC_W_W_W */
    1045,
    /* CRC_W_B_W */
    1048,
    /* CRC_W_D_W */
    1051,
    /* CRC_W_H_W */
    1054,
    /* CRC_W_W_W */
    1057,
    /* CSRRD */
    1060,
    /* CSRWR */
    1062,
    /* CSRXCHG */
    1065,
    /* CTO_D */
    1069,
    /* CTO_W */
    1071,
    /* CTZ_D */
    1073,
    /* CTZ_W */
    1075,
    /* DBAR */
    1077,
    /* DBCL */
    1078,
    /* DIV_D */
    1079,
    /* DIV_DU */
    1082,
    /* DIV_W */
    1085,
    /* DIV_WU */
    1088,
    /* ERTN */
    1091,
    /* EXT_W_B */
    1091,
    /* EXT_W_H */
    1093,
    /* FABS_D */
    1095,
    /* FABS_S */
    1097,
    /* FADD_D */
    1099,
    /* FADD_S */
    1102,
    /* FCLASS_D */
    1105,
    /* FCLASS_S */
    1107,
    /* FCMP_CAF_D */
    1109,
    /* FCMP_CAF_S */
    1112,
    /* FCMP_CEQ_D */
    1115,
    /* FCMP_CEQ_S */
    1118,
    /* FCMP_CLE_D */
    1121,
    /* FCMP_CLE_S */
    1124,
    /* FCMP_CLT_D */
    1127,
    /* FCMP_CLT_S */
    1130,
    /* FCMP_CNE_D */
    1133,
    /* FCMP_CNE_S */
    1136,
    /* FCMP_COR_D */
    1139,
    /* FCMP_COR_S */
    1142,
    /* FCMP_CUEQ_D */
    1145,
    /* FCMP_CUEQ_S */
    1148,
    /* FCMP_CULE_D */
    1151,
    /* FCMP_CULE_S */
    1154,
    /* FCMP_CULT_D */
    1157,
    /* FCMP_CULT_S */
    1160,
    /* FCMP_CUNE_D */
    1163,
    /* FCMP_CUNE_S */
    1166,
    /* FCMP_CUN_D */
    1169,
    /* FCMP_CUN_S */
    1172,
    /* FCMP_SAF_D */
    1175,
    /* FCMP_SAF_S */
    1178,
    /* FCMP_SEQ_D */
    1181,
    /* FCMP_SEQ_S */
    1184,
    /* FCMP_SLE_D */
    1187,
    /* FCMP_SLE_S */
    1190,
    /* FCMP_SLT_D */
    1193,
    /* FCMP_SLT_S */
    1196,
    /* FCMP_SNE_D */
    1199,
    /* FCMP_SNE_S */
    1202,
    /* FCMP_SOR_D */
    1205,
    /* FCMP_SOR_S */
    1208,
    /* FCMP_SUEQ_D */
    1211,
    /* FCMP_SUEQ_S */
    1214,
    /* FCMP_SULE_D */
    1217,
    /* FCMP_SULE_S */
    1220,
    /* FCMP_SULT_D */
    1223,
    /* FCMP_SULT_S */
    1226,
    /* FCMP_SUNE_D */
    1229,
    /* FCMP_SUNE_S */
    1232,
    /* FCMP_SUN_D */
    1235,
    /* FCMP_SUN_S */
    1238,
    /* FCOPYSIGN_D */
    1241,
    /* FCOPYSIGN_S */
    1244,
    /* FCVT_D_S */
    1247,
    /* FCVT_S_D */
    1249,
    /* FDIV_D */
    1251,
    /* FDIV_S */
    1254,
    /* FFINT_D_L */
    1257,
    /* FFINT_D_W */
    1259,
    /* FFINT_S_L */
    1261,
    /* FFINT_S_W */
    1263,
    /* FLDGT_D */
    1265,
    /* FLDGT_S */
    1268,
    /* FLDLE_D */
    1271,
    /* FLDLE_S */
    1274,
    /* FLDX_D */
    1277,
    /* FLDX_S */
    1280,
    /* FLD_D */
    1283,
    /* FLD_S */
    1286,
    /* FLOGB_D */
    1289,
    /* FLOGB_S */
    1291,
    /* FMADD_D */
    1293,
    /* FMADD_S */
    1297,
    /* FMAXA_D */
    1301,
    /* FMAXA_S */
    1304,
    /* FMAX_D */
    1307,
    /* FMAX_S */
    1310,
    /* FMINA_D */
    1313,
    /* FMINA_S */
    1316,
    /* FMIN_D */
    1319,
    /* FMIN_S */
    1322,
    /* FMOV_D */
    1325,
    /* FMOV_S */
    1327,
    /* FMSUB_D */
    1329,
    /* FMSUB_S */
    1333,
    /* FMUL_D */
    1337,
    /* FMUL_S */
    1340,
    /* FNEG_D */
    1343,
    /* FNEG_S */
    1345,
    /* FNMADD_D */
    1347,
    /* FNMADD_S */
    1351,
    /* FNMSUB_D */
    1355,
    /* FNMSUB_S */
    1359,
    /* FRECIP_D */
    1363,
    /* FRECIP_S */
    1365,
    /* FRINT_D */
    1367,
    /* FRINT_S */
    1369,
    /* FRSQRT_D */
    1371,
    /* FRSQRT_S */
    1373,
    /* FSCALEB_D */
    1375,
    /* FSCALEB_S */
    1378,
    /* FSEL_D */
    1381,
    /* FSEL_S */
    1385,
    /* FSQRT_D */
    1389,
    /* FSQRT_S */
    1391,
    /* FSTGT_D */
    1393,
    /* FSTGT_S */
    1396,
    /* FSTLE_D */
    1399,
    /* FSTLE_S */
    1402,
    /* FSTX_D */
    1405,
    /* FSTX_S */
    1408,
    /* FST_D */
    1411,
    /* FST_S */
    1414,
    /* FSUB_D */
    1417,
    /* FSUB_S */
    1420,
    /* FTINTRM_L_D */
    1423,
    /* FTINTRM_L_S */
    1425,
    /* FTINTRM_W_D */
    1427,
    /* FTINTRM_W_S */
    1429,
    /* FTINTRNE_L_D */
    1431,
    /* FTINTRNE_L_S */
    1433,
    /* FTINTRNE_W_D */
    1435,
    /* FTINTRNE_W_S */
    1437,
    /* FTINTRP_L_D */
    1439,
    /* FTINTRP_L_S */
    1441,
    /* FTINTRP_W_D */
    1443,
    /* FTINTRP_W_S */
    1445,
    /* FTINTRZ_L_D */
    1447,
    /* FTINTRZ_L_S */
    1449,
    /* FTINTRZ_W_D */
    1451,
    /* FTINTRZ_W_S */
    1453,
    /* FTINT_L_D */
    1455,
    /* FTINT_L_S */
    1457,
    /* FTINT_W_D */
    1459,
    /* FTINT_W_S */
    1461,
    /* IBAR */
    1463,
    /* IDLE */
    1464,
    /* INVTLB */
    1465,
    /* IOCSRRD_B */
    1468,
    /* IOCSRRD_D */
    1470,
    /* IOCSRRD_H */
    1472,
    /* IOCSRRD_W */
    1474,
    /* IOCSRWR_B */
    1476,
    /* IOCSRWR_D */
    1478,
    /* IOCSRWR_H */
    1480,
    /* IOCSRWR_W */
    1482,
    /* JIRL */
    1484,
    /* LDDIR */
    1487,
    /* LDGT_B */
    1490,
    /* LDGT_D */
    1493,
    /* LDGT_H */
    1496,
    /* LDGT_W */
    1499,
    /* LDLE_B */
    1502,
    /* LDLE_D */
    1505,
    /* LDLE_H */
    1508,
    /* LDLE_W */
    1511,
    /* LDPTE */
    1514,
    /* LDPTR_D */
    1516,
    /* LDPTR_W */
    1519,
    /* LDX_B */
    1522,
    /* LDX_BU */
    1525,
    /* LDX_D */
    1528,
    /* LDX_H */
    1531,
    /* LDX_HU */
    1534,
    /* LDX_W */
    1537,
    /* LDX_WU */
    1540,
    /* LD_B */
    1543,
    /* LD_BU */
    1546,
    /* LD_D */
    1549,
    /* LD_H */
    1552,
    /* LD_HU */
    1555,
    /* LD_W */
    1558,
    /* LD_WU */
    1561,
    /* LL_D */
    1564,
    /* LL_W */
    1567,
    /* LU12I_W */
    1570,
    /* LU32I_D */
    1572,
    /* LU52I_D */
    1575,
    /* MASKEQZ */
    1578,
    /* MASKNEZ */
    1581,
    /* MOD_D */
    1584,
    /* MOD_DU */
    1587,
    /* MOD_W */
    1590,
    /* MOD_WU */
    1593,
    /* MOVCF2FR_S */
    1596,
    /* MOVCF2GR */
    1598,
    /* MOVFCSR2GR */
    1600,
    /* MOVFR2CF_S */
    1602,
    /* MOVFR2GR_D */
    1604,
    /* MOVFR2GR_S */
    1606,
    /* MOVFR2GR_S_64 */
    1608,
    /* MOVFRH2GR_S */
    1610,
    /* MOVGR2CF */
    1612,
    /* MOVGR2FCSR */
    1614,
    /* MOVGR2FRH_W */
    1616,
    /* MOVGR2FR_D */
    1619,
    /* MOVGR2FR_W */
    1621,
    /* MOVGR2FR_W_64 */
    1623,
    /* MULH_D */
    1625,
    /* MULH_DU */
    1628,
    /* MULH_W */
    1631,
    /* MULH_WU */
    1634,
    /* MULW_D_W */
    1637,
    /* MULW_D_WU */
    1640,
    /* MUL_D */
    1643,
    /* MUL_W */
    1646,
    /* NOR */
    1649,
    /* OR */
    1652,
    /* ORI */
    1655,
    /* ORN */
    1658,
    /* PCADDI */
    1661,
    /* PCADDU12I */
    1663,
    /* PCADDU18I */
    1665,
    /* PCALAU12I */
    1667,
    /* PRELD */
    1669,
    /* PRELDX */
    1672,
    /* RDTIMEH_W */
    1675,
    /* RDTIMEL_W */
    1677,
    /* RDTIME_D */
    1679,
    /* REVB_2H */
    1681,
    /* REVB_2W */
    1683,
    /* REVB_4H */
    1685,
    /* REVB_D */
    1687,
    /* REVH_2W */
    1689,
    /* REVH_D */
    1691,
    /* ROTRI_D */
    1693,
    /* ROTRI_W */
    1696,
    /* ROTR_D */
    1699,
    /* ROTR_W */
    1702,
    /* SC_D */
    1705,
    /* SC_W */
    1709,
    /* SLLI_D */
    1713,
    /* SLLI_W */
    1716,
    /* SLL_D */
    1719,
    /* SLL_W */
    1722,
    /* SLT */
    1725,
    /* SLTI */
    1728,
    /* SLTU */
    1731,
    /* SLTUI */
    1734,
    /* SRAI_D */
    1737,
    /* SRAI_W */
    1740,
    /* SRA_D */
    1743,
    /* SRA_W */
    1746,
    /* SRLI_D */
    1749,
    /* SRLI_W */
    1752,
    /* SRL_D */
    1755,
    /* SRL_W */
    1758,
    /* STGT_B */
    1761,
    /* STGT_D */
    1764,
    /* STGT_H */
    1767,
    /* STGT_W */
    1770,
    /* STLE_B */
    1773,
    /* STLE_D */
    1776,
    /* STLE_H */
    1779,
    /* STLE_W */
    1782,
    /* STPTR_D */
    1785,
    /* STPTR_W */
    1788,
    /* STX_B */
    1791,
    /* STX_D */
    1794,
    /* STX_H */
    1797,
    /* STX_W */
    1800,
    /* ST_B */
    1803,
    /* ST_D */
    1806,
    /* ST_H */
    1809,
    /* ST_W */
    1812,
    /* SUB_D */
    1815,
    /* SUB_W */
    1818,
    /* SYSCALL */
    1821,
    /* TLBCLR */
    1822,
    /* TLBFILL */
    1822,
    /* TLBFLUSH */
    1822,
    /* TLBRD */
    1822,
    /* TLBSRCH */
    1822,
    /* TLBWR */
    1822,
    /* XOR */
    1822,
    /* XORI */
    1825,
  };

  using namespace OpTypes;
  const int8_t OpcodeOperandTypes[] = {
    
    /* PHI */
    -1, 
    /* INLINEASM */
    /* INLINEASM_BR */
    /* CFI_INSTRUCTION */
    i32imm, 
    /* EH_LABEL */
    i32imm, 
    /* GC_LABEL */
    i32imm, 
    /* ANNOTATION_LABEL */
    i32imm, 
    /* KILL */
    /* EXTRACT_SUBREG */
    -1, -1, i32imm, 
    /* INSERT_SUBREG */
    -1, -1, -1, i32imm, 
    /* IMPLICIT_DEF */
    -1, 
    /* SUBREG_TO_REG */
    -1, -1, -1, i32imm, 
    /* COPY_TO_REGCLASS */
    -1, -1, i32imm, 
    /* DBG_VALUE */
    /* DBG_VALUE_LIST */
    /* DBG_INSTR_REF */
    /* DBG_PHI */
    /* DBG_LABEL */
    -1, 
    /* REG_SEQUENCE */
    -1, -1, 
    /* COPY */
    -1, -1, 
    /* BUNDLE */
    /* LIFETIME_START */
    i32imm, 
    /* LIFETIME_END */
    i32imm, 
    /* PSEUDO_PROBE */
    i64imm, i64imm, i8imm, i32imm, 
    /* ARITH_FENCE */
    -1, -1, 
    /* STACKMAP */
    i64imm, i32imm, 
    /* FENTRY_CALL */
    /* PATCHPOINT */
    -1, i64imm, i32imm, -1, i32imm, i32imm, 
    /* LOAD_STACK_GUARD */
    -1, 
    /* PREALLOCATED_SETUP */
    i32imm, 
    /* PREALLOCATED_ARG */
    -1, i32imm, i32imm, 
    /* STATEPOINT */
    /* LOCAL_ESCAPE */
    -1, i32imm, 
    /* FAULTING_OP */
    -1, 
    /* PATCHABLE_OP */
    /* PATCHABLE_FUNCTION_ENTER */
    /* PATCHABLE_RET */
    /* PATCHABLE_FUNCTION_EXIT */
    /* PATCHABLE_TAIL_CALL */
    /* PATCHABLE_EVENT_CALL */
    -1, -1, 
    /* PATCHABLE_TYPED_EVENT_CALL */
    -1, -1, -1, 
    /* ICALL_BRANCH_FUNNEL */
    /* MEMBARRIER */
    /* G_ASSERT_SEXT */
    type0, type0, untyped_imm_0, 
    /* G_ASSERT_ZEXT */
    type0, type0, untyped_imm_0, 
    /* G_ASSERT_ALIGN */
    type0, type0, untyped_imm_0, 
    /* G_ADD */
    type0, type0, type0, 
    /* G_SUB */
    type0, type0, type0, 
    /* G_MUL */
    type0, type0, type0, 
    /* G_SDIV */
    type0, type0, type0, 
    /* G_UDIV */
    type0, type0, type0, 
    /* G_SREM */
    type0, type0, type0, 
    /* G_UREM */
    type0, type0, type0, 
    /* G_SDIVREM */
    type0, type0, type0, type0, 
    /* G_UDIVREM */
    type0, type0, type0, type0, 
    /* G_AND */
    type0, type0, type0, 
    /* G_OR */
    type0, type0, type0, 
    /* G_XOR */
    type0, type0, type0, 
    /* G_IMPLICIT_DEF */
    type0, 
    /* G_PHI */
    type0, 
    /* G_FRAME_INDEX */
    type0, -1, 
    /* G_GLOBAL_VALUE */
    type0, -1, 
    /* G_EXTRACT */
    type0, type1, untyped_imm_0, 
    /* G_UNMERGE_VALUES */
    type0, type1, 
    /* G_INSERT */
    type0, type0, type1, untyped_imm_0, 
    /* G_MERGE_VALUES */
    type0, type1, 
    /* G_BUILD_VECTOR */
    type0, type1, 
    /* G_BUILD_VECTOR_TRUNC */
    type0, type1, 
    /* G_CONCAT_VECTORS */
    type0, type1, 
    /* G_PTRTOINT */
    type0, type1, 
    /* G_INTTOPTR */
    type0, type1, 
    /* G_BITCAST */
    type0, type1, 
    /* G_FREEZE */
    type0, type0, 
    /* G_INTRINSIC_FPTRUNC_ROUND */
    type0, type1, i32imm, 
    /* G_INTRINSIC_TRUNC */
    type0, type0, 
    /* G_INTRINSIC_ROUND */
    type0, type0, 
    /* G_INTRINSIC_LRINT */
    type0, type1, 
    /* G_INTRINSIC_ROUNDEVEN */
    type0, type0, 
    /* G_READCYCLECOUNTER */
    type0, 
    /* G_LOAD */
    type0, ptype1, 
    /* G_SEXTLOAD */
    type0, ptype1, 
    /* G_ZEXTLOAD */
    type0, ptype1, 
    /* G_INDEXED_LOAD */
    type0, ptype1, ptype1, type2, -1, 
    /* G_INDEXED_SEXTLOAD */
    type0, ptype1, ptype1, type2, -1, 
    /* G_INDEXED_ZEXTLOAD */
    type0, ptype1, ptype1, type2, -1, 
    /* G_STORE */
    type0, ptype1, 
    /* G_INDEXED_STORE */
    ptype0, type1, ptype0, ptype2, -1, 
    /* G_ATOMIC_CMPXCHG_WITH_SUCCESS */
    type0, type1, type2, type0, type0, 
    /* G_ATOMIC_CMPXCHG */
    type0, ptype1, type0, type0, 
    /* G_ATOMICRMW_XCHG */
    type0, ptype1, type0, 
    /* G_ATOMICRMW_ADD */
    type0, ptype1, type0, 
    /* G_ATOMICRMW_SUB */
    type0, ptype1, type0, 
    /* G_ATOMICRMW_AND */
    type0, ptype1, type0, 
    /* G_ATOMICRMW_NAND */
    type0, ptype1, type0, 
    /* G_ATOMICRMW_OR */
    type0, ptype1, type0, 
    /* G_ATOMICRMW_XOR */
    type0, ptype1, type0, 
    /* G_ATOMICRMW_MAX */
    type0, ptype1, type0, 
    /* G_ATOMICRMW_MIN */
    type0, ptype1, type0, 
    /* G_ATOMICRMW_UMAX */
    type0, ptype1, type0, 
    /* G_ATOMICRMW_UMIN */
    type0, ptype1, type0, 
    /* G_ATOMICRMW_FADD */
    type0, ptype1, type0, 
    /* G_ATOMICRMW_FSUB */
    type0, ptype1, type0, 
    /* G_ATOMICRMW_FMAX */
    type0, ptype1, type0, 
    /* G_ATOMICRMW_FMIN */
    type0, ptype1, type0, 
    /* G_ATOMICRMW_UINC_WRAP */
    type0, ptype1, type0, 
    /* G_ATOMICRMW_UDEC_WRAP */
    type0, ptype1, type0, 
    /* G_FENCE */
    i32imm, i32imm, 
    /* G_BRCOND */
    type0, -1, 
    /* G_BRINDIRECT */
    type0, 
    /* G_INVOKE_REGION_START */
    /* G_INTRINSIC */
    -1, 
    /* G_INTRINSIC_W_SIDE_EFFECTS */
    -1, 
    /* G_ANYEXT */
    type0, type1, 
    /* G_TRUNC */
    type0, type1, 
    /* G_CONSTANT */
    type0, -1, 
    /* G_FCONSTANT */
    type0, -1, 
    /* G_VASTART */
    type0, 
    /* G_VAARG */
    type0, type1, -1, 
    /* G_SEXT */
    type0, type1, 
    /* G_SEXT_INREG */
    type0, type0, untyped_imm_0, 
    /* G_ZEXT */
    type0, type1, 
    /* G_SHL */
    type0, type0, type1, 
    /* G_LSHR */
    type0, type0, type1, 
    /* G_ASHR */
    type0, type0, type1, 
    /* G_FSHL */
    type0, type0, type0, type1, 
    /* G_FSHR */
    type0, type0, type0, type1, 
    /* G_ROTR */
    type0, type0, type1, 
    /* G_ROTL */
    type0, type0, type1, 
    /* G_ICMP */
    type0, -1, type1, type1, 
    /* G_FCMP */
    type0, -1, type1, type1, 
    /* G_SELECT */
    type0, type1, type0, type0, 
    /* G_UADDO */
    type0, type1, type0, type0, 
    /* G_UADDE */
    type0, type1, type0, type0, type1, 
    /* G_USUBO */
    type0, type1, type0, type0, 
    /* G_USUBE */
    type0, type1, type0, type0, type1, 
    /* G_SADDO */
    type0, type1, type0, type0, 
    /* G_SADDE */
    type0, type1, type0, type0, type1, 
    /* G_SSUBO */
    type0, type1, type0, type0, 
    /* G_SSUBE */
    type0, type1, type0, type0, type1, 
    /* G_UMULO */
    type0, type1, type0, type0, 
    /* G_SMULO */
    type0, type1, type0, type0, 
    /* G_UMULH */
    type0, type0, type0, 
    /* G_SMULH */
    type0, type0, type0, 
    /* G_UADDSAT */
    type0, type0, type0, 
    /* G_SADDSAT */
    type0, type0, type0, 
    /* G_USUBSAT */
    type0, type0, type0, 
    /* G_SSUBSAT */
    type0, type0, type0, 
    /* G_USHLSAT */
    type0, type0, type1, 
    /* G_SSHLSAT */
    type0, type0, type1, 
    /* G_SMULFIX */
    type0, type0, type0, untyped_imm_0, 
    /* G_UMULFIX */
    type0, type0, type0, untyped_imm_0, 
    /* G_SMULFIXSAT */
    type0, type0, type0, untyped_imm_0, 
    /* G_UMULFIXSAT */
    type0, type0, type0, untyped_imm_0, 
    /* G_SDIVFIX */
    type0, type0, type0, untyped_imm_0, 
    /* G_UDIVFIX */
    type0, type0, type0, untyped_imm_0, 
    /* G_SDIVFIXSAT */
    type0, type0, type0, untyped_imm_0, 
    /* G_UDIVFIXSAT */
    type0, type0, type0, untyped_imm_0, 
    /* G_FADD */
    type0, type0, type0, 
    /* G_FSUB */
    type0, type0, type0, 
    /* G_FMUL */
    type0, type0, type0, 
    /* G_FMA */
    type0, type0, type0, type0, 
    /* G_FMAD */
    type0, type0, type0, type0, 
    /* G_FDIV */
    type0, type0, type0, 
    /* G_FREM */
    type0, type0, type0, 
    /* G_FPOW */
    type0, type0, type0, 
    /* G_FPOWI */
    type0, type0, type1, 
    /* G_FEXP */
    type0, type0, 
    /* G_FEXP2 */
    type0, type0, 
    /* G_FLOG */
    type0, type0, 
    /* G_FLOG2 */
    type0, type0, 
    /* G_FLOG10 */
    type0, type0, 
    /* G_FNEG */
    type0, type0, 
    /* G_FPEXT */
    type0, type1, 
    /* G_FPTRUNC */
    type0, type1, 
    /* G_FPTOSI */
    type0, type1, 
    /* G_FPTOUI */
    type0, type1, 
    /* G_SITOFP */
    type0, type1, 
    /* G_UITOFP */
    type0, type1, 
    /* G_FABS */
    type0, type0, 
    /* G_FCOPYSIGN */
    type0, type0, type1, 
    /* G_IS_FPCLASS */
    type0, type1, -1, 
    /* G_FCANONICALIZE */
    type0, type0, 
    /* G_FMINNUM */
    type0, type0, type0, 
    /* G_FMAXNUM */
    type0, type0, type0, 
    /* G_FMINNUM_IEEE */
    type0, type0, type0, 
    /* G_FMAXNUM_IEEE */
    type0, type0, type0, 
    /* G_FMINIMUM */
    type0, type0, type0, 
    /* G_FMAXIMUM */
    type0, type0, type0, 
    /* G_PTR_ADD */
    ptype0, ptype0, type1, 
    /* G_PTRMASK */
    ptype0, ptype0, type1, 
    /* G_SMIN */
    type0, type0, type0, 
    /* G_SMAX */
    type0, type0, type0, 
    /* G_UMIN */
    type0, type0, type0, 
    /* G_UMAX */
    type0, type0, type0, 
    /* G_ABS */
    type0, type0, 
    /* G_LROUND */
    type0, type1, 
    /* G_LLROUND */
    type0, type1, 
    /* G_BR */
    -1, 
    /* G_BRJT */
    ptype0, -1, type1, 
    /* G_INSERT_VECTOR_ELT */
    type0, type0, type1, type2, 
    /* G_EXTRACT_VECTOR_ELT */
    type0, type1, type2, 
    /* G_SHUFFLE_VECTOR */
    type0, type1, type1, -1, 
    /* G_CTTZ */
    type0, type1, 
    /* G_CTTZ_ZERO_UNDEF */
    type0, type1, 
    /* G_CTLZ */
    type0, type1, 
    /* G_CTLZ_ZERO_UNDEF */
    type0, type1, 
    /* G_CTPOP */
    type0, type1, 
    /* G_BSWAP */
    type0, type0, 
    /* G_BITREVERSE */
    type0, type0, 
    /* G_FCEIL */
    type0, type0, 
    /* G_FCOS */
    type0, type0, 
    /* G_FSIN */
    type0, type0, 
    /* G_FSQRT */
    type0, type0, 
    /* G_FFLOOR */
    type0, type0, 
    /* G_FRINT */
    type0, type0, 
    /* G_FNEARBYINT */
    type0, type0, 
    /* G_ADDRSPACE_CAST */
    type0, type1, 
    /* G_BLOCK_ADDR */
    type0, -1, 
    /* G_JUMP_TABLE */
    type0, -1, 
    /* G_DYN_STACKALLOC */
    ptype0, type1, i32imm, 
    /* G_STRICT_FADD */
    type0, type0, type0, 
    /* G_STRICT_FSUB */
    type0, type0, type0, 
    /* G_STRICT_FMUL */
    type0, type0, type0, 
    /* G_STRICT_FDIV */
    type0, type0, type0, 
    /* G_STRICT_FREM */
    type0, type0, type0, 
    /* G_STRICT_FMA */
    type0, type0, type0, type0, 
    /* G_STRICT_FSQRT */
    type0, type0, 
    /* G_READ_REGISTER */
    type0, -1, 
    /* G_WRITE_REGISTER */
    -1, type0, 
    /* G_MEMCPY */
    ptype0, ptype1, type2, untyped_imm_0, 
    /* G_MEMCPY_INLINE */
    ptype0, ptype1, type2, 
    /* G_MEMMOVE */
    ptype0, ptype1, type2, untyped_imm_0, 
    /* G_MEMSET */
    ptype0, type1, type2, untyped_imm_0, 
    /* G_BZERO */
    ptype0, type1, untyped_imm_0, 
    /* G_VECREDUCE_SEQ_FADD */
    type0, type1, type2, 
    /* G_VECREDUCE_SEQ_FMUL */
    type0, type1, type2, 
    /* G_VECREDUCE_FADD */
    type0, type1, 
    /* G_VECREDUCE_FMUL */
    type0, type1, 
    /* G_VECREDUCE_FMAX */
    type0, type1, 
    /* G_VECREDUCE_FMIN */
    type0, type1, 
    /* G_VECREDUCE_ADD */
    type0, type1, 
    /* G_VECREDUCE_MUL */
    type0, type1, 
    /* G_VECREDUCE_AND */
    type0, type1, 
    /* G_VECREDUCE_OR */
    type0, type1, 
    /* G_VECREDUCE_XOR */
    type0, type1, 
    /* G_VECREDUCE_SMAX */
    type0, type1, 
    /* G_VECREDUCE_SMIN */
    type0, type1, 
    /* G_VECREDUCE_UMAX */
    type0, type1, 
    /* G_VECREDUCE_UMIN */
    type0, type1, 
    /* G_SBFX */
    type0, type0, type1, type1, 
    /* G_UBFX */
    type0, type0, type1, type1, 
    /* ADJCALLSTACKDOWN */
    i32imm, i32imm, 
    /* ADJCALLSTACKUP */
    i32imm, i32imm, 
    /* PseudoAtomicLoadAdd32 */
    GPR, GPR, GPR, GPR, grlenimm, 
    /* PseudoAtomicLoadAnd32 */
    GPR, GPR, GPR, GPR, grlenimm, 
    /* PseudoAtomicLoadNand32 */
    GPR, GPR, GPR, GPR, grlenimm, 
    /* PseudoAtomicLoadNand64 */
    GPR, GPR, GPR, GPR, grlenimm, 
    /* PseudoAtomicLoadOr32 */
    GPR, GPR, GPR, GPR, grlenimm, 
    /* PseudoAtomicLoadSub32 */
    GPR, GPR, GPR, GPR, grlenimm, 
    /* PseudoAtomicLoadXor32 */
    GPR, GPR, GPR, GPR, grlenimm, 
    /* PseudoAtomicStoreD */
    GPR, GPR, GPR, 
    /* PseudoAtomicStoreW */
    GPR, GPR, GPR, 
    /* PseudoAtomicSwap32 */
    GPR, GPR, GPR, GPR, grlenimm, 
    /* PseudoBR */
    simm26_b, 
    /* PseudoBRIND */
    GPR, simm16_lsl2, 
    /* PseudoB_TAIL */
    simm26_b, 
    /* PseudoCALL */
    simm26_symbol, 
    /* PseudoCALLIndirect */
    GPR, 
    /* PseudoCmpXchg32 */
    GPR, GPR, GPR, GPR, GPR, 
    /* PseudoCmpXchg64 */
    GPR, GPR, GPR, GPR, GPR, 
    /* PseudoJIRL_CALL */
    GPR, simm16_lsl2, 
    /* PseudoJIRL_TAIL */
    GPR, simm16_lsl2, 
    /* PseudoLA_ABS */
    GPR, bare_symbol, 
    /* PseudoLA_ABS_LARGE */
    GPR, GPR, bare_symbol, 
    /* PseudoLA_GOT */
    GPR, bare_symbol, 
    /* PseudoLA_GOT_LARGE */
    GPR, GPR, bare_symbol, 
    /* PseudoLA_PCREL */
    GPR, bare_symbol, 
    /* PseudoLA_PCREL_LARGE */
    GPR, GPR, bare_symbol, 
    /* PseudoLA_TLS_GD */
    GPR, bare_symbol, 
    /* PseudoLA_TLS_GD_LARGE */
    GPR, GPR, bare_symbol, 
    /* PseudoLA_TLS_IE */
    GPR, bare_symbol, 
    /* PseudoLA_TLS_IE_LARGE */
    GPR, GPR, bare_symbol, 
    /* PseudoLA_TLS_LD */
    GPR, bare_symbol, 
    /* PseudoLA_TLS_LD_LARGE */
    GPR, GPR, bare_symbol, 
    /* PseudoLA_TLS_LE */
    GPR, bare_symbol, 
    /* PseudoLD_CFR */
    CFR, GPR, grlenimm, 
    /* PseudoLI_D */
    GPR, grlenimm, 
    /* PseudoLI_W */
    GPR, imm32, 
    /* PseudoMaskedAtomicLoadAdd32 */
    GPR, GPR, GPR, GPR, GPR, grlenimm, 
    /* PseudoMaskedAtomicLoadMax32 */
    GPR, GPR, GPR, GPR, GPR, GPR, grlenimm, grlenimm, 
    /* PseudoMaskedAtomicLoadMin32 */
    GPR, GPR, GPR, GPR, GPR, GPR, grlenimm, grlenimm, 
    /* PseudoMaskedAtomicLoadNand32 */
    GPR, GPR, GPR, GPR, GPR, grlenimm, 
    /* PseudoMaskedAtomicLoadSub32 */
    GPR, GPR, GPR, GPR, GPR, grlenimm, 
    /* PseudoMaskedAtomicLoadUMax32 */
    GPR, GPR, GPR, GPR, GPR, GPR, grlenimm, 
    /* PseudoMaskedAtomicLoadUMin32 */
    GPR, GPR, GPR, GPR, GPR, GPR, grlenimm, 
    /* PseudoMaskedAtomicSwap32 */
    GPR, GPR, GPR, GPR, GPR, grlenimm, 
    /* PseudoMaskedCmpXchg32 */
    GPR, GPR, GPR, GPR, GPR, GPR, grlenimm, 
    /* PseudoRET */
    /* PseudoST_CFR */
    CFR, GPR, grlenimm, 
    /* PseudoTAIL */
    simm26_symbol, 
    /* PseudoTAILIndirect */
    GPRT, 
    /* PseudoUNIMP */
    /* RDFCSR */
    GPR, uimm2, 
    /* WRFCSR */
    uimm2, GPR, 
    /* ADDI_D */
    GPR, GPR, simm12_addlike, 
    /* ADDI_W */
    GPR, GPR, simm12_addlike, 
    /* ADDU16I_D */
    GPR, GPR, simm16, 
    /* ADD_D */
    GPR, GPR, GPR, 
    /* ADD_W */
    GPR, GPR, GPR, 
    /* ALSL_D */
    GPR, GPR, GPR, uimm2_plus1, 
    /* ALSL_W */
    GPR, GPR, GPR, uimm2_plus1, 
    /* ALSL_WU */
    GPR, GPR, GPR, uimm2_plus1, 
    /* AMADD_D */
    GPR, GPR, GPRMemAtomic, 
    /* AMADD_DB_D */
    GPR, GPR, GPRMemAtomic, 
    /* AMADD_DB_W */
    GPR, GPR, GPRMemAtomic, 
    /* AMADD_W */
    GPR, GPR, GPRMemAtomic, 
    /* AMAND_D */
    GPR, GPR, GPRMemAtomic, 
    /* AMAND_DB_D */
    GPR, GPR, GPRMemAtomic, 
    /* AMAND_DB_W */
    GPR, GPR, GPRMemAtomic, 
    /* AMAND_W */
    GPR, GPR, GPRMemAtomic, 
    /* AMMAX_D */
    GPR, GPR, GPRMemAtomic, 
    /* AMMAX_DB_D */
    GPR, GPR, GPRMemAtomic, 
    /* AMMAX_DB_DU */
    GPR, GPR, GPRMemAtomic, 
    /* AMMAX_DB_W */
    GPR, GPR, GPRMemAtomic, 
    /* AMMAX_DB_WU */
    GPR, GPR, GPRMemAtomic, 
    /* AMMAX_DU */
    GPR, GPR, GPRMemAtomic, 
    /* AMMAX_W */
    GPR, GPR, GPRMemAtomic, 
    /* AMMAX_WU */
    GPR, GPR, GPRMemAtomic, 
    /* AMMIN_D */
    GPR, GPR, GPRMemAtomic, 
    /* AMMIN_DB_D */
    GPR, GPR, GPRMemAtomic, 
    /* AMMIN_DB_DU */
    GPR, GPR, GPRMemAtomic, 
    /* AMMIN_DB_W */
    GPR, GPR, GPRMemAtomic, 
    /* AMMIN_DB_WU */
    GPR, GPR, GPRMemAtomic, 
    /* AMMIN_DU */
    GPR, GPR, GPRMemAtomic, 
    /* AMMIN_W */
    GPR, GPR, GPRMemAtomic, 
    /* AMMIN_WU */
    GPR, GPR, GPRMemAtomic, 
    /* AMOR_D */
    GPR, GPR, GPRMemAtomic, 
    /* AMOR_DB_D */
    GPR, GPR, GPRMemAtomic, 
    /* AMOR_DB_W */
    GPR, GPR, GPRMemAtomic, 
    /* AMOR_W */
    GPR, GPR, GPRMemAtomic, 
    /* AMSWAP_D */
    GPR, GPR, GPRMemAtomic, 
    /* AMSWAP_DB_D */
    GPR, GPR, GPRMemAtomic, 
    /* AMSWAP_DB_W */
    GPR, GPR, GPRMemAtomic, 
    /* AMSWAP_W */
    GPR, GPR, GPRMemAtomic, 
    /* AMXOR_D */
    GPR, GPR, GPRMemAtomic, 
    /* AMXOR_DB_D */
    GPR, GPR, GPRMemAtomic, 
    /* AMXOR_DB_W */
    GPR, GPR, GPRMemAtomic, 
    /* AMXOR_W */
    GPR, GPR, GPRMemAtomic, 
    /* AND */
    GPR, GPR, GPR, 
    /* ANDI */
    GPR, GPR, uimm12, 
    /* ANDN */
    GPR, GPR, GPR, 
    /* ASRTGT_D */
    GPR, GPR, 
    /* ASRTLE_D */
    GPR, GPR, 
    /* B */
    simm26_b, 
    /* BCEQZ */
    CFR, simm21_lsl2, 
    /* BCNEZ */
    CFR, simm21_lsl2, 
    /* BEQ */
    GPR, GPR, simm16_lsl2_br, 
    /* BEQZ */
    GPR, simm21_lsl2, 
    /* BGE */
    GPR, GPR, simm16_lsl2_br, 
    /* BGEU */
    GPR, GPR, simm16_lsl2_br, 
    /* BITREV_4B */
    GPR, GPR, 
    /* BITREV_8B */
    GPR, GPR, 
    /* BITREV_D */
    GPR, GPR, 
    /* BITREV_W */
    GPR, GPR, 
    /* BL */
    simm26_symbol, 
    /* BLT */
    GPR, GPR, simm16_lsl2_br, 
    /* BLTU */
    GPR, GPR, simm16_lsl2_br, 
    /* BNE */
    GPR, GPR, simm16_lsl2_br, 
    /* BNEZ */
    GPR, simm21_lsl2, 
    /* BREAK */
    uimm15, 
    /* BSTRINS_D */
    GPR, GPR, GPR, uimm6, uimm6, 
    /* BSTRINS_W */
    GPR, GPR, GPR, uimm5, uimm5, 
    /* BSTRPICK_D */
    GPR, GPR, uimm6, uimm6, 
    /* BSTRPICK_W */
    GPR, GPR, uimm5, uimm5, 
    /* BYTEPICK_D */
    GPR, GPR, GPR, uimm3, 
    /* BYTEPICK_W */
    GPR, GPR, GPR, uimm2, 
    /* CACOP */
    uimm5, GPR, simm12, 
    /* CLO_D */
    GPR, GPR, 
    /* CLO_W */
    GPR, GPR, 
    /* CLZ_D */
    GPR, GPR, 
    /* CLZ_W */
    GPR, GPR, 
    /* CPUCFG */
    GPR, GPR, 
    /* CRCC_W_B_W */
    GPR, GPR, GPR, 
    /* CRCC_W_D_W */
    GPR, GPR, GPR, 
    /* CRCC_W_H_W */
    GPR, GPR, GPR, 
    /* CRCC_W_W_W */
    GPR, GPR, GPR, 
    /* CRC_W_B_W */
    GPR, GPR, GPR, 
    /* CRC_W_D_W */
    GPR, GPR, GPR, 
    /* CRC_W_H_W */
    GPR, GPR, GPR, 
    /* CRC_W_W_W */
    GPR, GPR, GPR, 
    /* CSRRD */
    GPR, uimm14, 
    /* CSRWR */
    GPR, GPR, uimm14, 
    /* CSRXCHG */
    GPR, GPR, GPR, uimm14, 
    /* CTO_D */
    GPR, GPR, 
    /* CTO_W */
    GPR, GPR, 
    /* CTZ_D */
    GPR, GPR, 
    /* CTZ_W */
    GPR, GPR, 
    /* DBAR */
    uimm15, 
    /* DBCL */
    uimm15, 
    /* DIV_D */
    GPR, GPR, GPR, 
    /* DIV_DU */
    GPR, GPR, GPR, 
    /* DIV_W */
    GPR, GPR, GPR, 
    /* DIV_WU */
    GPR, GPR, GPR, 
    /* ERTN */
    /* EXT_W_B */
    GPR, GPR, 
    /* EXT_W_H */
    GPR, GPR, 
    /* FABS_D */
    FPR64, FPR64, 
    /* FABS_S */
    FPR32, FPR32, 
    /* FADD_D */
    FPR64, FPR64, FPR64, 
    /* FADD_S */
    FPR32, FPR32, FPR32, 
    /* FCLASS_D */
    FPR64, FPR64, 
    /* FCLASS_S */
    FPR32, FPR32, 
    /* FCMP_CAF_D */
    CFR, FPR64, FPR64, 
    /* FCMP_CAF_S */
    CFR, FPR32, FPR32, 
    /* FCMP_CEQ_D */
    CFR, FPR64, FPR64, 
    /* FCMP_CEQ_S */
    CFR, FPR32, FPR32, 
    /* FCMP_CLE_D */
    CFR, FPR64, FPR64, 
    /* FCMP_CLE_S */
    CFR, FPR32, FPR32, 
    /* FCMP_CLT_D */
    CFR, FPR64, FPR64, 
    /* FCMP_CLT_S */
    CFR, FPR32, FPR32, 
    /* FCMP_CNE_D */
    CFR, FPR64, FPR64, 
    /* FCMP_CNE_S */
    CFR, FPR32, FPR32, 
    /* FCMP_COR_D */
    CFR, FPR64, FPR64, 
    /* FCMP_COR_S */
    CFR, FPR32, FPR32, 
    /* FCMP_CUEQ_D */
    CFR, FPR64, FPR64, 
    /* FCMP_CUEQ_S */
    CFR, FPR32, FPR32, 
    /* FCMP_CULE_D */
    CFR, FPR64, FPR64, 
    /* FCMP_CULE_S */
    CFR, FPR32, FPR32, 
    /* FCMP_CULT_D */
    CFR, FPR64, FPR64, 
    /* FCMP_CULT_S */
    CFR, FPR32, FPR32, 
    /* FCMP_CUNE_D */
    CFR, FPR64, FPR64, 
    /* FCMP_CUNE_S */
    CFR, FPR32, FPR32, 
    /* FCMP_CUN_D */
    CFR, FPR64, FPR64, 
    /* FCMP_CUN_S */
    CFR, FPR32, FPR32, 
    /* FCMP_SAF_D */
    CFR, FPR64, FPR64, 
    /* FCMP_SAF_S */
    CFR, FPR32, FPR32, 
    /* FCMP_SEQ_D */
    CFR, FPR64, FPR64, 
    /* FCMP_SEQ_S */
    CFR, FPR32, FPR32, 
    /* FCMP_SLE_D */
    CFR, FPR64, FPR64, 
    /* FCMP_SLE_S */
    CFR, FPR32, FPR32, 
    /* FCMP_SLT_D */
    CFR, FPR64, FPR64, 
    /* FCMP_SLT_S */
    CFR, FPR32, FPR32, 
    /* FCMP_SNE_D */
    CFR, FPR64, FPR64, 
    /* FCMP_SNE_S */
    CFR, FPR32, FPR32, 
    /* FCMP_SOR_D */
    CFR, FPR64, FPR64, 
    /* FCMP_SOR_S */
    CFR, FPR32, FPR32, 
    /* FCMP_SUEQ_D */
    CFR, FPR64, FPR64, 
    /* FCMP_SUEQ_S */
    CFR, FPR32, FPR32, 
    /* FCMP_SULE_D */
    CFR, FPR64, FPR64, 
    /* FCMP_SULE_S */
    CFR, FPR32, FPR32, 
    /* FCMP_SULT_D */
    CFR, FPR64, FPR64, 
    /* FCMP_SULT_S */
    CFR, FPR32, FPR32, 
    /* FCMP_SUNE_D */
    CFR, FPR64, FPR64, 
    /* FCMP_SUNE_S */
    CFR, FPR32, FPR32, 
    /* FCMP_SUN_D */
    CFR, FPR64, FPR64, 
    /* FCMP_SUN_S */
    CFR, FPR32, FPR32, 
    /* FCOPYSIGN_D */
    FPR64, FPR64, FPR64, 
    /* FCOPYSIGN_S */
    FPR32, FPR32, FPR32, 
    /* FCVT_D_S */
    FPR64, FPR32, 
    /* FCVT_S_D */
    FPR32, FPR64, 
    /* FDIV_D */
    FPR64, FPR64, FPR64, 
    /* FDIV_S */
    FPR32, FPR32, FPR32, 
    /* FFINT_D_L */
    FPR64, FPR64, 
    /* FFINT_D_W */
    FPR64, FPR32, 
    /* FFINT_S_L */
    FPR32, FPR64, 
    /* FFINT_S_W */
    FPR32, FPR32, 
    /* FLDGT_D */
    FPR64, GPR, GPR, 
    /* FLDGT_S */
    FPR32, GPR, GPR, 
    /* FLDLE_D */
    FPR64, GPR, GPR, 
    /* FLDLE_S */
    FPR32, GPR, GPR, 
    /* FLDX_D */
    FPR64, GPR, GPR, 
    /* FLDX_S */
    FPR32, GPR, GPR, 
    /* FLD_D */
    FPR64, GPR, simm12, 
    /* FLD_S */
    FPR32, GPR, simm12, 
    /* FLOGB_D */
    FPR64, FPR64, 
    /* FLOGB_S */
    FPR32, FPR32, 
    /* FMADD_D */
    FPR64, FPR64, FPR64, FPR64, 
    /* FMADD_S */
    FPR32, FPR32, FPR32, FPR32, 
    /* FMAXA_D */
    FPR64, FPR64, FPR64, 
    /* FMAXA_S */
    FPR32, FPR32, FPR32, 
    /* FMAX_D */
    FPR64, FPR64, FPR64, 
    /* FMAX_S */
    FPR32, FPR32, FPR32, 
    /* FMINA_D */
    FPR64, FPR64, FPR64, 
    /* FMINA_S */
    FPR32, FPR32, FPR32, 
    /* FMIN_D */
    FPR64, FPR64, FPR64, 
    /* FMIN_S */
    FPR32, FPR32, FPR32, 
    /* FMOV_D */
    FPR64, FPR64, 
    /* FMOV_S */
    FPR32, FPR32, 
    /* FMSUB_D */
    FPR64, FPR64, FPR64, FPR64, 
    /* FMSUB_S */
    FPR32, FPR32, FPR32, FPR32, 
    /* FMUL_D */
    FPR64, FPR64, FPR64, 
    /* FMUL_S */
    FPR32, FPR32, FPR32, 
    /* FNEG_D */
    FPR64, FPR64, 
    /* FNEG_S */
    FPR32, FPR32, 
    /* FNMADD_D */
    FPR64, FPR64, FPR64, FPR64, 
    /* FNMADD_S */
    FPR32, FPR32, FPR32, FPR32, 
    /* FNMSUB_D */
    FPR64, FPR64, FPR64, FPR64, 
    /* FNMSUB_S */
    FPR32, FPR32, FPR32, FPR32, 
    /* FRECIP_D */
    FPR64, FPR64, 
    /* FRECIP_S */
    FPR32, FPR32, 
    /* FRINT_D */
    FPR64, FPR64, 
    /* FRINT_S */
    FPR32, FPR32, 
    /* FRSQRT_D */
    FPR64, FPR64, 
    /* FRSQRT_S */
    FPR32, FPR32, 
    /* FSCALEB_D */
    FPR64, FPR64, FPR64, 
    /* FSCALEB_S */
    FPR32, FPR32, FPR32, 
    /* FSEL_D */
    FPR64, FPR64, FPR64, CFR, 
    /* FSEL_S */
    FPR32, FPR32, FPR32, CFR, 
    /* FSQRT_D */
    FPR64, FPR64, 
    /* FSQRT_S */
    FPR32, FPR32, 
    /* FSTGT_D */
    FPR64, GPR, GPR, 
    /* FSTGT_S */
    FPR32, GPR, GPR, 
    /* FSTLE_D */
    FPR64, GPR, GPR, 
    /* FSTLE_S */
    FPR32, GPR, GPR, 
    /* FSTX_D */
    FPR64, GPR, GPR, 
    /* FSTX_S */
    FPR32, GPR, GPR, 
    /* FST_D */
    FPR64, GPR, simm12, 
    /* FST_S */
    FPR32, GPR, simm12, 
    /* FSUB_D */
    FPR64, FPR64, FPR64, 
    /* FSUB_S */
    FPR32, FPR32, FPR32, 
    /* FTINTRM_L_D */
    FPR64, FPR64, 
    /* FTINTRM_L_S */
    FPR64, FPR32, 
    /* FTINTRM_W_D */
    FPR32, FPR64, 
    /* FTINTRM_W_S */
    FPR32, FPR32, 
    /* FTINTRNE_L_D */
    FPR64, FPR64, 
    /* FTINTRNE_L_S */
    FPR64, FPR32, 
    /* FTINTRNE_W_D */
    FPR32, FPR64, 
    /* FTINTRNE_W_S */
    FPR32, FPR32, 
    /* FTINTRP_L_D */
    FPR64, FPR64, 
    /* FTINTRP_L_S */
    FPR64, FPR32, 
    /* FTINTRP_W_D */
    FPR32, FPR64, 
    /* FTINTRP_W_S */
    FPR32, FPR32, 
    /* FTINTRZ_L_D */
    FPR64, FPR64, 
    /* FTINTRZ_L_S */
    FPR64, FPR32, 
    /* FTINTRZ_W_D */
    FPR32, FPR64, 
    /* FTINTRZ_W_S */
    FPR32, FPR32, 
    /* FTINT_L_D */
    FPR64, FPR64, 
    /* FTINT_L_S */
    FPR64, FPR32, 
    /* FTINT_W_D */
    FPR32, FPR64, 
    /* FTINT_W_S */
    FPR32, FPR32, 
    /* IBAR */
    uimm15, 
    /* IDLE */
    uimm15, 
    /* INVTLB */
    GPR, GPR, uimm5, 
    /* IOCSRRD_B */
    GPR, GPR, 
    /* IOCSRRD_D */
    GPR, GPR, 
    /* IOCSRRD_H */
    GPR, GPR, 
    /* IOCSRRD_W */
    GPR, GPR, 
    /* IOCSRWR_B */
    GPR, GPR, 
    /* IOCSRWR_D */
    GPR, GPR, 
    /* IOCSRWR_H */
    GPR, GPR, 
    /* IOCSRWR_W */
    GPR, GPR, 
    /* JIRL */
    GPR, GPR, simm16_lsl2, 
    /* LDDIR */
    GPR, GPR, uimm8, 
    /* LDGT_B */
    GPR, GPR, GPR, 
    /* LDGT_D */
    GPR, GPR, GPR, 
    /* LDGT_H */
    GPR, GPR, GPR, 
    /* LDGT_W */
    GPR, GPR, GPR, 
    /* LDLE_B */
    GPR, GPR, GPR, 
    /* LDLE_D */
    GPR, GPR, GPR, 
    /* LDLE_H */
    GPR, GPR, GPR, 
    /* LDLE_W */
    GPR, GPR, GPR, 
    /* LDPTE */
    GPR, uimm8, 
    /* LDPTR_D */
    GPR, GPR, simm14_lsl2, 
    /* LDPTR_W */
    GPR, GPR, simm14_lsl2, 
    /* LDX_B */
    GPR, GPR, GPR, 
    /* LDX_BU */
    GPR, GPR, GPR, 
    /* LDX_D */
    GPR, GPR, GPR, 
    /* LDX_H */
    GPR, GPR, GPR, 
    /* LDX_HU */
    GPR, GPR, GPR, 
    /* LDX_W */
    GPR, GPR, GPR, 
    /* LDX_WU */
    GPR, GPR, GPR, 
    /* LD_B */
    GPR, GPR, simm12_addlike, 
    /* LD_BU */
    GPR, GPR, simm12_addlike, 
    /* LD_D */
    GPR, GPR, simm12_addlike, 
    /* LD_H */
    GPR, GPR, simm12_addlike, 
    /* LD_HU */
    GPR, GPR, simm12_addlike, 
    /* LD_W */
    GPR, GPR, simm12_addlike, 
    /* LD_WU */
    GPR, GPR, simm12_addlike, 
    /* LL_D */
    GPR, GPR, simm14_lsl2, 
    /* LL_W */
    GPR, GPR, simm14_lsl2, 
    /* LU12I_W */
    GPR, simm20_lu12iw, 
    /* LU32I_D */
    GPR, GPR, simm20_lu32id, 
    /* LU52I_D */
    GPR, GPR, simm12_lu52id, 
    /* MASKEQZ */
    GPR, GPR, GPR, 
    /* MASKNEZ */
    GPR, GPR, GPR, 
    /* MOD_D */
    GPR, GPR, GPR, 
    /* MOD_DU */
    GPR, GPR, GPR, 
    /* MOD_W */
    GPR, GPR, GPR, 
    /* MOD_WU */
    GPR, GPR, GPR, 
    /* MOVCF2FR_S */
    FPR32, CFR, 
    /* MOVCF2GR */
    GPR, CFR, 
    /* MOVFCSR2GR */
    GPR, FCSR, 
    /* MOVFR2CF_S */
    CFR, FPR32, 
    /* MOVFR2GR_D */
    GPR, FPR64, 
    /* MOVFR2GR_S */
    GPR, FPR32, 
    /* MOVFR2GR_S_64 */
    GPR, FPR64, 
    /* MOVFRH2GR_S */
    GPR, FPR64, 
    /* MOVGR2CF */
    CFR, GPR, 
    /* MOVGR2FCSR */
    FCSR, GPR, 
    /* MOVGR2FRH_W */
    FPR64, FPR64, GPR, 
    /* MOVGR2FR_D */
    FPR64, GPR, 
    /* MOVGR2FR_W */
    FPR32, GPR, 
    /* MOVGR2FR_W_64 */
    FPR64, GPR, 
    /* MULH_D */
    GPR, GPR, GPR, 
    /* MULH_DU */
    GPR, GPR, GPR, 
    /* MULH_W */
    GPR, GPR, GPR, 
    /* MULH_WU */
    GPR, GPR, GPR, 
    /* MULW_D_W */
    GPR, GPR, GPR, 
    /* MULW_D_WU */
    GPR, GPR, GPR, 
    /* MUL_D */
    GPR, GPR, GPR, 
    /* MUL_W */
    GPR, GPR, GPR, 
    /* NOR */
    GPR, GPR, GPR, 
    /* OR */
    GPR, GPR, GPR, 
    /* ORI */
    GPR, GPR, uimm12_ori, 
    /* ORN */
    GPR, GPR, GPR, 
    /* PCADDI */
    GPR, simm20, 
    /* PCADDU12I */
    GPR, simm20, 
    /* PCADDU18I */
    GPR, simm20, 
    /* PCALAU12I */
    GPR, simm20_pcalau12i, 
    /* PRELD */
    uimm5, GPR, simm12, 
    /* PRELDX */
    uimm5, GPR, GPR, 
    /* RDTIMEH_W */
    GPR, GPR, 
    /* RDTIMEL_W */
    GPR, GPR, 
    /* RDTIME_D */
    GPR, GPR, 
    /* REVB_2H */
    GPR, GPR, 
    /* REVB_2W */
    GPR, GPR, 
    /* REVB_4H */
    GPR, GPR, 
    /* REVB_D */
    GPR, GPR, 
    /* REVH_2W */
    GPR, GPR, 
    /* REVH_D */
    GPR, GPR, 
    /* ROTRI_D */
    GPR, GPR, uimm6, 
    /* ROTRI_W */
    GPR, GPR, uimm5, 
    /* ROTR_D */
    GPR, GPR, GPR, 
    /* ROTR_W */
    GPR, GPR, GPR, 
    /* SC_D */
    GPR, GPR, GPR, simm14_lsl2, 
    /* SC_W */
    GPR, GPR, GPR, simm14_lsl2, 
    /* SLLI_D */
    GPR, GPR, uimm6, 
    /* SLLI_W */
    GPR, GPR, uimm5, 
    /* SLL_D */
    GPR, GPR, GPR, 
    /* SLL_W */
    GPR, GPR, GPR, 
    /* SLT */
    GPR, GPR, GPR, 
    /* SLTI */
    GPR, GPR, simm12, 
    /* SLTU */
    GPR, GPR, GPR, 
    /* SLTUI */
    GPR, GPR, simm12, 
    /* SRAI_D */
    GPR, GPR, uimm6, 
    /* SRAI_W */
    GPR, GPR, uimm5, 
    /* SRA_D */
    GPR, GPR, GPR, 
    /* SRA_W */
    GPR, GPR, GPR, 
    /* SRLI_D */
    GPR, GPR, uimm6, 
    /* SRLI_W */
    GPR, GPR, uimm5, 
    /* SRL_D */
    GPR, GPR, GPR, 
    /* SRL_W */
    GPR, GPR, GPR, 
    /* STGT_B */
    GPR, GPR, GPR, 
    /* STGT_D */
    GPR, GPR, GPR, 
    /* STGT_H */
    GPR, GPR, GPR, 
    /* STGT_W */
    GPR, GPR, GPR, 
    /* STLE_B */
    GPR, GPR, GPR, 
    /* STLE_D */
    GPR, GPR, GPR, 
    /* STLE_H */
    GPR, GPR, GPR, 
    /* STLE_W */
    GPR, GPR, GPR, 
    /* STPTR_D */
    GPR, GPR, simm14_lsl2, 
    /* STPTR_W */
    GPR, GPR, simm14_lsl2, 
    /* STX_B */
    GPR, GPR, GPR, 
    /* STX_D */
    GPR, GPR, GPR, 
    /* STX_H */
    GPR, GPR, GPR, 
    /* STX_W */
    GPR, GPR, GPR, 
    /* ST_B */
    GPR, GPR, simm12_addlike, 
    /* ST_D */
    GPR, GPR, simm12_addlike, 
    /* ST_H */
    GPR, GPR, simm12_addlike, 
    /* ST_W */
    GPR, GPR, simm12_addlike, 
    /* SUB_D */
    GPR, GPR, GPR, 
    /* SUB_W */
    GPR, GPR, GPR, 
    /* SYSCALL */
    uimm15, 
    /* TLBCLR */
    /* TLBFILL */
    /* TLBFLUSH */
    /* TLBRD */
    /* TLBSRCH */
    /* TLBWR */
    /* XOR */
    GPR, GPR, GPR, 
    /* XORI */
    GPR, GPR, uimm12, 
  };
  return OpcodeOperandTypes[Offsets[Opcode] + OpIdx];
}
} // end namespace LoongArch
} // end namespace llvm
#endif // GET_INSTRINFO_OPERAND_TYPE

#ifdef GET_INSTRINFO_MEM_OPERAND_SIZE
#undef GET_INSTRINFO_MEM_OPERAND_SIZE
namespace llvm {
namespace LoongArch {
LLVM_READONLY
static int getMemOperandSize(int OpType) {
  switch (OpType) {
  default: return 0;
  }
}
} // end namespace LoongArch
} // end namespace llvm
#endif // GET_INSTRINFO_MEM_OPERAND_SIZE

#ifdef GET_INSTRINFO_LOGICAL_OPERAND_SIZE_MAP
#undef GET_INSTRINFO_LOGICAL_OPERAND_SIZE_MAP
namespace llvm {
namespace LoongArch {
LLVM_READONLY static unsigned
getLogicalOperandSize(uint16_t Opcode, uint16_t LogicalOpIdx) {
  return LogicalOpIdx;
}
LLVM_READONLY static inline unsigned
getLogicalOperandIdx(uint16_t Opcode, uint16_t LogicalOpIdx) {
  auto S = 0U;
  for (auto i = 0U; i < LogicalOpIdx; ++i)
    S += getLogicalOperandSize(Opcode, i);
  return S;
}
} // end namespace LoongArch
} // end namespace llvm
#endif // GET_INSTRINFO_LOGICAL_OPERAND_SIZE_MAP

#ifdef GET_INSTRINFO_LOGICAL_OPERAND_TYPE_MAP
#undef GET_INSTRINFO_LOGICAL_OPERAND_TYPE_MAP
namespace llvm {
namespace LoongArch {
LLVM_READONLY static int
getLogicalOperandType(uint16_t Opcode, uint16_t LogicalOpIdx) {
  return -1;
}
} // end namespace LoongArch
} // end namespace llvm
#endif // GET_INSTRINFO_LOGICAL_OPERAND_TYPE_MAP

#ifdef GET_INSTRINFO_MC_HELPER_DECLS
#undef GET_INSTRINFO_MC_HELPER_DECLS

namespace llvm {
class MCInst;
class FeatureBitset;

namespace LoongArch_MC {

void verifyInstructionPredicates(unsigned Opcode, const FeatureBitset &Features);

} // end namespace LoongArch_MC
} // end namespace llvm

#endif // GET_INSTRINFO_MC_HELPER_DECLS

#ifdef GET_INSTRINFO_MC_HELPERS
#undef GET_INSTRINFO_MC_HELPERS

namespace llvm {
namespace LoongArch_MC {

} // end namespace LoongArch_MC
} // end namespace llvm

#endif // GET_GENISTRINFO_MC_HELPERS

#ifdef ENABLE_INSTR_PREDICATE_VERIFIER
#undef ENABLE_INSTR_PREDICATE_VERIFIER
#include <sstream>

namespace llvm {
namespace LoongArch_MC {

// Bits for subtarget features that participate in instruction matching.
enum SubtargetFeatureBits : uint8_t {
  Feature_IsLA64Bit = 10,
  Feature_IsLA32Bit = 9,
  Feature_HasBasicFBit = 1,
  Feature_HasBasicDBit = 0,
  Feature_HasExtLSXBit = 4,
  Feature_HasExtLASXBit = 2,
  Feature_HasExtLVZBit = 5,
  Feature_HasExtLBTBit = 3,
  Feature_HasLaGlobalWithPcrelBit = 7,
  Feature_HasLaGlobalWithAbsBit = 6,
  Feature_HasLaLocalWithAbsBit = 8,
};

#ifndef NDEBUG
static const char *SubtargetFeatureNames[] = {
  "Feature_HasBasicD",
  "Feature_HasBasicF",
  "Feature_HasExtLASX",
  "Feature_HasExtLBT",
  "Feature_HasExtLSX",
  "Feature_HasExtLVZ",
  "Feature_HasLaGlobalWithAbs",
  "Feature_HasLaGlobalWithPcrel",
  "Feature_HasLaLocalWithAbs",
  "Feature_IsLA32",
  "Feature_IsLA64",
  nullptr
};

#endif // NDEBUG

FeatureBitset computeAvailableFeatures(const FeatureBitset &FB) {
  FeatureBitset Features;
  if (FB[LoongArch::Feature64Bit])
    Features.set(Feature_IsLA64Bit);
  if (!FB[LoongArch::Feature64Bit])
    Features.set(Feature_IsLA32Bit);
  if (FB[LoongArch::FeatureBasicF])
    Features.set(Feature_HasBasicFBit);
  if (FB[LoongArch::FeatureBasicD])
    Features.set(Feature_HasBasicDBit);
  if (FB[LoongArch::FeatureExtLSX])
    Features.set(Feature_HasExtLSXBit);
  if (FB[LoongArch::FeatureExtLASX])
    Features.set(Feature_HasExtLASXBit);
  if (FB[LoongArch::FeatureExtLVZ])
    Features.set(Feature_HasExtLVZBit);
  if (FB[LoongArch::FeatureExtLBT])
    Features.set(Feature_HasExtLBTBit);
  if (FB[LoongArch::LaGlobalWithPcrel])
    Features.set(Feature_HasLaGlobalWithPcrelBit);
  if (FB[LoongArch::LaGlobalWithAbs])
    Features.set(Feature_HasLaGlobalWithAbsBit);
  if (FB[LoongArch::LaLocalWithAbs])
    Features.set(Feature_HasLaLocalWithAbsBit);
  return Features;
}

#ifndef NDEBUG
// Feature bitsets.
enum : uint8_t {
  CEFBS_None,
  CEFBS_HasBasicD,
  CEFBS_HasBasicF,
  CEFBS_IsLA64,
  CEFBS_HasBasicD_IsLA32,
  CEFBS_HasBasicD_IsLA64,
};

static constexpr FeatureBitset FeatureBitsets[] = {
  {}, // CEFBS_None
  {Feature_HasBasicDBit, },
  {Feature_HasBasicFBit, },
  {Feature_IsLA64Bit, },
  {Feature_HasBasicDBit, Feature_IsLA32Bit, },
  {Feature_HasBasicDBit, Feature_IsLA64Bit, },
};
#endif // NDEBUG

void verifyInstructionPredicates(
    unsigned Opcode, const FeatureBitset &Features) {
#ifndef NDEBUG
  static uint8_t RequiredFeaturesRefs[] = {
    CEFBS_None, // PHI = 0
    CEFBS_None, // INLINEASM = 1
    CEFBS_None, // INLINEASM_BR = 2
    CEFBS_None, // CFI_INSTRUCTION = 3
    CEFBS_None, // EH_LABEL = 4
    CEFBS_None, // GC_LABEL = 5
    CEFBS_None, // ANNOTATION_LABEL = 6
    CEFBS_None, // KILL = 7
    CEFBS_None, // EXTRACT_SUBREG = 8
    CEFBS_None, // INSERT_SUBREG = 9
    CEFBS_None, // IMPLICIT_DEF = 10
    CEFBS_None, // SUBREG_TO_REG = 11
    CEFBS_None, // COPY_TO_REGCLASS = 12
    CEFBS_None, // DBG_VALUE = 13
    CEFBS_None, // DBG_VALUE_LIST = 14
    CEFBS_None, // DBG_INSTR_REF = 15
    CEFBS_None, // DBG_PHI = 16
    CEFBS_None, // DBG_LABEL = 17
    CEFBS_None, // REG_SEQUENCE = 18
    CEFBS_None, // COPY = 19
    CEFBS_None, // BUNDLE = 20
    CEFBS_None, // LIFETIME_START = 21
    CEFBS_None, // LIFETIME_END = 22
    CEFBS_None, // PSEUDO_PROBE = 23
    CEFBS_None, // ARITH_FENCE = 24
    CEFBS_None, // STACKMAP = 25
    CEFBS_None, // FENTRY_CALL = 26
    CEFBS_None, // PATCHPOINT = 27
    CEFBS_None, // LOAD_STACK_GUARD = 28
    CEFBS_None, // PREALLOCATED_SETUP = 29
    CEFBS_None, // PREALLOCATED_ARG = 30
    CEFBS_None, // STATEPOINT = 31
    CEFBS_None, // LOCAL_ESCAPE = 32
    CEFBS_None, // FAULTING_OP = 33
    CEFBS_None, // PATCHABLE_OP = 34
    CEFBS_None, // PATCHABLE_FUNCTION_ENTER = 35
    CEFBS_None, // PATCHABLE_RET = 36
    CEFBS_None, // PATCHABLE_FUNCTION_EXIT = 37
    CEFBS_None, // PATCHABLE_TAIL_CALL = 38
    CEFBS_None, // PATCHABLE_EVENT_CALL = 39
    CEFBS_None, // PATCHABLE_TYPED_EVENT_CALL = 40
    CEFBS_None, // ICALL_BRANCH_FUNNEL = 41
    CEFBS_None, // MEMBARRIER = 42
    CEFBS_None, // G_ASSERT_SEXT = 43
    CEFBS_None, // G_ASSERT_ZEXT = 44
    CEFBS_None, // G_ASSERT_ALIGN = 45
    CEFBS_None, // G_ADD = 46
    CEFBS_None, // G_SUB = 47
    CEFBS_None, // G_MUL = 48
    CEFBS_None, // G_SDIV = 49
    CEFBS_None, // G_UDIV = 50
    CEFBS_None, // G_SREM = 51
    CEFBS_None, // G_UREM = 52
    CEFBS_None, // G_SDIVREM = 53
    CEFBS_None, // G_UDIVREM = 54
    CEFBS_None, // G_AND = 55
    CEFBS_None, // G_OR = 56
    CEFBS_None, // G_XOR = 57
    CEFBS_None, // G_IMPLICIT_DEF = 58
    CEFBS_None, // G_PHI = 59
    CEFBS_None, // G_FRAME_INDEX = 60
    CEFBS_None, // G_GLOBAL_VALUE = 61
    CEFBS_None, // G_EXTRACT = 62
    CEFBS_None, // G_UNMERGE_VALUES = 63
    CEFBS_None, // G_INSERT = 64
    CEFBS_None, // G_MERGE_VALUES = 65
    CEFBS_None, // G_BUILD_VECTOR = 66
    CEFBS_None, // G_BUILD_VECTOR_TRUNC = 67
    CEFBS_None, // G_CONCAT_VECTORS = 68
    CEFBS_None, // G_PTRTOINT = 69
    CEFBS_None, // G_INTTOPTR = 70
    CEFBS_None, // G_BITCAST = 71
    CEFBS_None, // G_FREEZE = 72
    CEFBS_None, // G_INTRINSIC_FPTRUNC_ROUND = 73
    CEFBS_None, // G_INTRINSIC_TRUNC = 74
    CEFBS_None, // G_INTRINSIC_ROUND = 75
    CEFBS_None, // G_INTRINSIC_LRINT = 76
    CEFBS_None, // G_INTRINSIC_ROUNDEVEN = 77
    CEFBS_None, // G_READCYCLECOUNTER = 78
    CEFBS_None, // G_LOAD = 79
    CEFBS_None, // G_SEXTLOAD = 80
    CEFBS_None, // G_ZEXTLOAD = 81
    CEFBS_None, // G_INDEXED_LOAD = 82
    CEFBS_None, // G_INDEXED_SEXTLOAD = 83
    CEFBS_None, // G_INDEXED_ZEXTLOAD = 84
    CEFBS_None, // G_STORE = 85
    CEFBS_None, // G_INDEXED_STORE = 86
    CEFBS_None, // G_ATOMIC_CMPXCHG_WITH_SUCCESS = 87
    CEFBS_None, // G_ATOMIC_CMPXCHG = 88
    CEFBS_None, // G_ATOMICRMW_XCHG = 89
    CEFBS_None, // G_ATOMICRMW_ADD = 90
    CEFBS_None, // G_ATOMICRMW_SUB = 91
    CEFBS_None, // G_ATOMICRMW_AND = 92
    CEFBS_None, // G_ATOMICRMW_NAND = 93
    CEFBS_None, // G_ATOMICRMW_OR = 94
    CEFBS_None, // G_ATOMICRMW_XOR = 95
    CEFBS_None, // G_ATOMICRMW_MAX = 96
    CEFBS_None, // G_ATOMICRMW_MIN = 97
    CEFBS_None, // G_ATOMICRMW_UMAX = 98
    CEFBS_None, // G_ATOMICRMW_UMIN = 99
    CEFBS_None, // G_ATOMICRMW_FADD = 100
    CEFBS_None, // G_ATOMICRMW_FSUB = 101
    CEFBS_None, // G_ATOMICRMW_FMAX = 102
    CEFBS_None, // G_ATOMICRMW_FMIN = 103
    CEFBS_None, // G_ATOMICRMW_UINC_WRAP = 104
    CEFBS_None, // G_ATOMICRMW_UDEC_WRAP = 105
    CEFBS_None, // G_FENCE = 106
    CEFBS_None, // G_BRCOND = 107
    CEFBS_None, // G_BRINDIRECT = 108
    CEFBS_None, // G_INVOKE_REGION_START = 109
    CEFBS_None, // G_INTRINSIC = 110
    CEFBS_None, // G_INTRINSIC_W_SIDE_EFFECTS = 111
    CEFBS_None, // G_ANYEXT = 112
    CEFBS_None, // G_TRUNC = 113
    CEFBS_None, // G_CONSTANT = 114
    CEFBS_None, // G_FCONSTANT = 115
    CEFBS_None, // G_VASTART = 116
    CEFBS_None, // G_VAARG = 117
    CEFBS_None, // G_SEXT = 118
    CEFBS_None, // G_SEXT_INREG = 119
    CEFBS_None, // G_ZEXT = 120
    CEFBS_None, // G_SHL = 121
    CEFBS_None, // G_LSHR = 122
    CEFBS_None, // G_ASHR = 123
    CEFBS_None, // G_FSHL = 124
    CEFBS_None, // G_FSHR = 125
    CEFBS_None, // G_ROTR = 126
    CEFBS_None, // G_ROTL = 127
    CEFBS_None, // G_ICMP = 128
    CEFBS_None, // G_FCMP = 129
    CEFBS_None, // G_SELECT = 130
    CEFBS_None, // G_UADDO = 131
    CEFBS_None, // G_UADDE = 132
    CEFBS_None, // G_USUBO = 133
    CEFBS_None, // G_USUBE = 134
    CEFBS_None, // G_SADDO = 135
    CEFBS_None, // G_SADDE = 136
    CEFBS_None, // G_SSUBO = 137
    CEFBS_None, // G_SSUBE = 138
    CEFBS_None, // G_UMULO = 139
    CEFBS_None, // G_SMULO = 140
    CEFBS_None, // G_UMULH = 141
    CEFBS_None, // G_SMULH = 142
    CEFBS_None, // G_UADDSAT = 143
    CEFBS_None, // G_SADDSAT = 144
    CEFBS_None, // G_USUBSAT = 145
    CEFBS_None, // G_SSUBSAT = 146
    CEFBS_None, // G_USHLSAT = 147
    CEFBS_None, // G_SSHLSAT = 148
    CEFBS_None, // G_SMULFIX = 149
    CEFBS_None, // G_UMULFIX = 150
    CEFBS_None, // G_SMULFIXSAT = 151
    CEFBS_None, // G_UMULFIXSAT = 152
    CEFBS_None, // G_SDIVFIX = 153
    CEFBS_None, // G_UDIVFIX = 154
    CEFBS_None, // G_SDIVFIXSAT = 155
    CEFBS_None, // G_UDIVFIXSAT = 156
    CEFBS_None, // G_FADD = 157
    CEFBS_None, // G_FSUB = 158
    CEFBS_None, // G_FMUL = 159
    CEFBS_None, // G_FMA = 160
    CEFBS_None, // G_FMAD = 161
    CEFBS_None, // G_FDIV = 162
    CEFBS_None, // G_FREM = 163
    CEFBS_None, // G_FPOW = 164
    CEFBS_None, // G_FPOWI = 165
    CEFBS_None, // G_FEXP = 166
    CEFBS_None, // G_FEXP2 = 167
    CEFBS_None, // G_FLOG = 168
    CEFBS_None, // G_FLOG2 = 169
    CEFBS_None, // G_FLOG10 = 170
    CEFBS_None, // G_FNEG = 171
    CEFBS_None, // G_FPEXT = 172
    CEFBS_None, // G_FPTRUNC = 173
    CEFBS_None, // G_FPTOSI = 174
    CEFBS_None, // G_FPTOUI = 175
    CEFBS_None, // G_SITOFP = 176
    CEFBS_None, // G_UITOFP = 177
    CEFBS_None, // G_FABS = 178
    CEFBS_None, // G_FCOPYSIGN = 179
    CEFBS_None, // G_IS_FPCLASS = 180
    CEFBS_None, // G_FCANONICALIZE = 181
    CEFBS_None, // G_FMINNUM = 182
    CEFBS_None, // G_FMAXNUM = 183
    CEFBS_None, // G_FMINNUM_IEEE = 184
    CEFBS_None, // G_FMAXNUM_IEEE = 185
    CEFBS_None, // G_FMINIMUM = 186
    CEFBS_None, // G_FMAXIMUM = 187
    CEFBS_None, // G_PTR_ADD = 188
    CEFBS_None, // G_PTRMASK = 189
    CEFBS_None, // G_SMIN = 190
    CEFBS_None, // G_SMAX = 191
    CEFBS_None, // G_UMIN = 192
    CEFBS_None, // G_UMAX = 193
    CEFBS_None, // G_ABS = 194
    CEFBS_None, // G_LROUND = 195
    CEFBS_None, // G_LLROUND = 196
    CEFBS_None, // G_BR = 197
    CEFBS_None, // G_BRJT = 198
    CEFBS_None, // G_INSERT_VECTOR_ELT = 199
    CEFBS_None, // G_EXTRACT_VECTOR_ELT = 200
    CEFBS_None, // G_SHUFFLE_VECTOR = 201
    CEFBS_None, // G_CTTZ = 202
    CEFBS_None, // G_CTTZ_ZERO_UNDEF = 203
    CEFBS_None, // G_CTLZ = 204
    CEFBS_None, // G_CTLZ_ZERO_UNDEF = 205
    CEFBS_None, // G_CTPOP = 206
    CEFBS_None, // G_BSWAP = 207
    CEFBS_None, // G_BITREVERSE = 208
    CEFBS_None, // G_FCEIL = 209
    CEFBS_None, // G_FCOS = 210
    CEFBS_None, // G_FSIN = 211
    CEFBS_None, // G_FSQRT = 212
    CEFBS_None, // G_FFLOOR = 213
    CEFBS_None, // G_FRINT = 214
    CEFBS_None, // G_FNEARBYINT = 215
    CEFBS_None, // G_ADDRSPACE_CAST = 216
    CEFBS_None, // G_BLOCK_ADDR = 217
    CEFBS_None, // G_JUMP_TABLE = 218
    CEFBS_None, // G_DYN_STACKALLOC = 219
    CEFBS_None, // G_STRICT_FADD = 220
    CEFBS_None, // G_STRICT_FSUB = 221
    CEFBS_None, // G_STRICT_FMUL = 222
    CEFBS_None, // G_STRICT_FDIV = 223
    CEFBS_None, // G_STRICT_FREM = 224
    CEFBS_None, // G_STRICT_FMA = 225
    CEFBS_None, // G_STRICT_FSQRT = 226
    CEFBS_None, // G_READ_REGISTER = 227
    CEFBS_None, // G_WRITE_REGISTER = 228
    CEFBS_None, // G_MEMCPY = 229
    CEFBS_None, // G_MEMCPY_INLINE = 230
    CEFBS_None, // G_MEMMOVE = 231
    CEFBS_None, // G_MEMSET = 232
    CEFBS_None, // G_BZERO = 233
    CEFBS_None, // G_VECREDUCE_SEQ_FADD = 234
    CEFBS_None, // G_VECREDUCE_SEQ_FMUL = 235
    CEFBS_None, // G_VECREDUCE_FADD = 236
    CEFBS_None, // G_VECREDUCE_FMUL = 237
    CEFBS_None, // G_VECREDUCE_FMAX = 238
    CEFBS_None, // G_VECREDUCE_FMIN = 239
    CEFBS_None, // G_VECREDUCE_ADD = 240
    CEFBS_None, // G_VECREDUCE_MUL = 241
    CEFBS_None, // G_VECREDUCE_AND = 242
    CEFBS_None, // G_VECREDUCE_OR = 243
    CEFBS_None, // G_VECREDUCE_XOR = 244
    CEFBS_None, // G_VECREDUCE_SMAX = 245
    CEFBS_None, // G_VECREDUCE_SMIN = 246
    CEFBS_None, // G_VECREDUCE_UMAX = 247
    CEFBS_None, // G_VECREDUCE_UMIN = 248
    CEFBS_None, // G_SBFX = 249
    CEFBS_None, // G_UBFX = 250
    CEFBS_None, // ADJCALLSTACKDOWN = 251
    CEFBS_None, // ADJCALLSTACKUP = 252
    CEFBS_None, // PseudoAtomicLoadAdd32 = 253
    CEFBS_None, // PseudoAtomicLoadAnd32 = 254
    CEFBS_None, // PseudoAtomicLoadNand32 = 255
    CEFBS_None, // PseudoAtomicLoadNand64 = 256
    CEFBS_None, // PseudoAtomicLoadOr32 = 257
    CEFBS_None, // PseudoAtomicLoadSub32 = 258
    CEFBS_None, // PseudoAtomicLoadXor32 = 259
    CEFBS_IsLA64, // PseudoAtomicStoreD = 260
    CEFBS_None, // PseudoAtomicStoreW = 261
    CEFBS_None, // PseudoAtomicSwap32 = 262
    CEFBS_None, // PseudoBR = 263
    CEFBS_None, // PseudoBRIND = 264
    CEFBS_None, // PseudoB_TAIL = 265
    CEFBS_None, // PseudoCALL = 266
    CEFBS_None, // PseudoCALLIndirect = 267
    CEFBS_None, // PseudoCmpXchg32 = 268
    CEFBS_None, // PseudoCmpXchg64 = 269
    CEFBS_None, // PseudoJIRL_CALL = 270
    CEFBS_None, // PseudoJIRL_TAIL = 271
    CEFBS_None, // PseudoLA_ABS = 272
    CEFBS_None, // PseudoLA_ABS_LARGE = 273
    CEFBS_None, // PseudoLA_GOT = 274
    CEFBS_IsLA64, // PseudoLA_GOT_LARGE = 275
    CEFBS_None, // PseudoLA_PCREL = 276
    CEFBS_IsLA64, // PseudoLA_PCREL_LARGE = 277
    CEFBS_None, // PseudoLA_TLS_GD = 278
    CEFBS_IsLA64, // PseudoLA_TLS_GD_LARGE = 279
    CEFBS_None, // PseudoLA_TLS_IE = 280
    CEFBS_IsLA64, // PseudoLA_TLS_IE_LARGE = 281
    CEFBS_None, // PseudoLA_TLS_LD = 282
    CEFBS_IsLA64, // PseudoLA_TLS_LD_LARGE = 283
    CEFBS_None, // PseudoLA_TLS_LE = 284
    CEFBS_HasBasicF, // PseudoLD_CFR = 285
    CEFBS_IsLA64, // PseudoLI_D = 286
    CEFBS_None, // PseudoLI_W = 287
    CEFBS_None, // PseudoMaskedAtomicLoadAdd32 = 288
    CEFBS_None, // PseudoMaskedAtomicLoadMax32 = 289
    CEFBS_None, // PseudoMaskedAtomicLoadMin32 = 290
    CEFBS_None, // PseudoMaskedAtomicLoadNand32 = 291
    CEFBS_None, // PseudoMaskedAtomicLoadSub32 = 292
    CEFBS_None, // PseudoMaskedAtomicLoadUMax32 = 293
    CEFBS_None, // PseudoMaskedAtomicLoadUMin32 = 294
    CEFBS_None, // PseudoMaskedAtomicSwap32 = 295
    CEFBS_None, // PseudoMaskedCmpXchg32 = 296
    CEFBS_None, // PseudoRET = 297
    CEFBS_HasBasicF, // PseudoST_CFR = 298
    CEFBS_None, // PseudoTAIL = 299
    CEFBS_None, // PseudoTAILIndirect = 300
    CEFBS_None, // PseudoUNIMP = 301
    CEFBS_HasBasicF, // RDFCSR = 302
    CEFBS_HasBasicF, // WRFCSR = 303
    CEFBS_IsLA64, // ADDI_D = 304
    CEFBS_None, // ADDI_W = 305
    CEFBS_IsLA64, // ADDU16I_D = 306
    CEFBS_IsLA64, // ADD_D = 307
    CEFBS_None, // ADD_W = 308
    CEFBS_IsLA64, // ALSL_D = 309
    CEFBS_None, // ALSL_W = 310
    CEFBS_IsLA64, // ALSL_WU = 311
    CEFBS_IsLA64, // AMADD_D = 312
    CEFBS_IsLA64, // AMADD_DB_D = 313
    CEFBS_IsLA64, // AMADD_DB_W = 314
    CEFBS_IsLA64, // AMADD_W = 315
    CEFBS_IsLA64, // AMAND_D = 316
    CEFBS_IsLA64, // AMAND_DB_D = 317
    CEFBS_IsLA64, // AMAND_DB_W = 318
    CEFBS_IsLA64, // AMAND_W = 319
    CEFBS_IsLA64, // AMMAX_D = 320
    CEFBS_IsLA64, // AMMAX_DB_D = 321
    CEFBS_IsLA64, // AMMAX_DB_DU = 322
    CEFBS_IsLA64, // AMMAX_DB_W = 323
    CEFBS_IsLA64, // AMMAX_DB_WU = 324
    CEFBS_IsLA64, // AMMAX_DU = 325
    CEFBS_IsLA64, // AMMAX_W = 326
    CEFBS_IsLA64, // AMMAX_WU = 327
    CEFBS_IsLA64, // AMMIN_D = 328
    CEFBS_IsLA64, // AMMIN_DB_D = 329
    CEFBS_IsLA64, // AMMIN_DB_DU = 330
    CEFBS_IsLA64, // AMMIN_DB_W = 331
    CEFBS_IsLA64, // AMMIN_DB_WU = 332
    CEFBS_IsLA64, // AMMIN_DU = 333
    CEFBS_IsLA64, // AMMIN_W = 334
    CEFBS_IsLA64, // AMMIN_WU = 335
    CEFBS_IsLA64, // AMOR_D = 336
    CEFBS_IsLA64, // AMOR_DB_D = 337
    CEFBS_IsLA64, // AMOR_DB_W = 338
    CEFBS_IsLA64, // AMOR_W = 339
    CEFBS_IsLA64, // AMSWAP_D = 340
    CEFBS_IsLA64, // AMSWAP_DB_D = 341
    CEFBS_IsLA64, // AMSWAP_DB_W = 342
    CEFBS_IsLA64, // AMSWAP_W = 343
    CEFBS_IsLA64, // AMXOR_D = 344
    CEFBS_IsLA64, // AMXOR_DB_D = 345
    CEFBS_IsLA64, // AMXOR_DB_W = 346
    CEFBS_IsLA64, // AMXOR_W = 347
    CEFBS_None, // AND = 348
    CEFBS_None, // ANDI = 349
    CEFBS_None, // ANDN = 350
    CEFBS_IsLA64, // ASRTGT_D = 351
    CEFBS_IsLA64, // ASRTLE_D = 352
    CEFBS_None, // B = 353
    CEFBS_HasBasicF, // BCEQZ = 354
    CEFBS_HasBasicF, // BCNEZ = 355
    CEFBS_None, // BEQ = 356
    CEFBS_None, // BEQZ = 357
    CEFBS_None, // BGE = 358
    CEFBS_None, // BGEU = 359
    CEFBS_None, // BITREV_4B = 360
    CEFBS_IsLA64, // BITREV_8B = 361
    CEFBS_IsLA64, // BITREV_D = 362
    CEFBS_None, // BITREV_W = 363
    CEFBS_None, // BL = 364
    CEFBS_None, // BLT = 365
    CEFBS_None, // BLTU = 366
    CEFBS_None, // BNE = 367
    CEFBS_None, // BNEZ = 368
    CEFBS_None, // BREAK = 369
    CEFBS_IsLA64, // BSTRINS_D = 370
    CEFBS_None, // BSTRINS_W = 371
    CEFBS_IsLA64, // BSTRPICK_D = 372
    CEFBS_None, // BSTRPICK_W = 373
    CEFBS_IsLA64, // BYTEPICK_D = 374
    CEFBS_None, // BYTEPICK_W = 375
    CEFBS_None, // CACOP = 376
    CEFBS_IsLA64, // CLO_D = 377
    CEFBS_None, // CLO_W = 378
    CEFBS_IsLA64, // CLZ_D = 379
    CEFBS_None, // CLZ_W = 380
    CEFBS_None, // CPUCFG = 381
    CEFBS_IsLA64, // CRCC_W_B_W = 382
    CEFBS_IsLA64, // CRCC_W_D_W = 383
    CEFBS_IsLA64, // CRCC_W_H_W = 384
    CEFBS_IsLA64, // CRCC_W_W_W = 385
    CEFBS_IsLA64, // CRC_W_B_W = 386
    CEFBS_IsLA64, // CRC_W_D_W = 387
    CEFBS_IsLA64, // CRC_W_H_W = 388
    CEFBS_IsLA64, // CRC_W_W_W = 389
    CEFBS_None, // CSRRD = 390
    CEFBS_None, // CSRWR = 391
    CEFBS_None, // CSRXCHG = 392
    CEFBS_IsLA64, // CTO_D = 393
    CEFBS_None, // CTO_W = 394
    CEFBS_IsLA64, // CTZ_D = 395
    CEFBS_None, // CTZ_W = 396
    CEFBS_None, // DBAR = 397
    CEFBS_None, // DBCL = 398
    CEFBS_IsLA64, // DIV_D = 399
    CEFBS_IsLA64, // DIV_DU = 400
    CEFBS_None, // DIV_W = 401
    CEFBS_None, // DIV_WU = 402
    CEFBS_None, // ERTN = 403
    CEFBS_None, // EXT_W_B = 404
    CEFBS_None, // EXT_W_H = 405
    CEFBS_HasBasicD, // FABS_D = 406
    CEFBS_HasBasicF, // FABS_S = 407
    CEFBS_HasBasicD, // FADD_D = 408
    CEFBS_HasBasicF, // FADD_S = 409
    CEFBS_HasBasicD, // FCLASS_D = 410
    CEFBS_HasBasicF, // FCLASS_S = 411
    CEFBS_HasBasicD, // FCMP_CAF_D = 412
    CEFBS_HasBasicF, // FCMP_CAF_S = 413
    CEFBS_HasBasicD, // FCMP_CEQ_D = 414
    CEFBS_HasBasicF, // FCMP_CEQ_S = 415
    CEFBS_HasBasicD, // FCMP_CLE_D = 416
    CEFBS_HasBasicF, // FCMP_CLE_S = 417
    CEFBS_HasBasicD, // FCMP_CLT_D = 418
    CEFBS_HasBasicF, // FCMP_CLT_S = 419
    CEFBS_HasBasicD, // FCMP_CNE_D = 420
    CEFBS_HasBasicF, // FCMP_CNE_S = 421
    CEFBS_HasBasicD, // FCMP_COR_D = 422
    CEFBS_HasBasicF, // FCMP_COR_S = 423
    CEFBS_HasBasicD, // FCMP_CUEQ_D = 424
    CEFBS_HasBasicF, // FCMP_CUEQ_S = 425
    CEFBS_HasBasicD, // FCMP_CULE_D = 426
    CEFBS_HasBasicF, // FCMP_CULE_S = 427
    CEFBS_HasBasicD, // FCMP_CULT_D = 428
    CEFBS_HasBasicF, // FCMP_CULT_S = 429
    CEFBS_HasBasicD, // FCMP_CUNE_D = 430
    CEFBS_HasBasicF, // FCMP_CUNE_S = 431
    CEFBS_HasBasicD, // FCMP_CUN_D = 432
    CEFBS_HasBasicF, // FCMP_CUN_S = 433
    CEFBS_HasBasicD, // FCMP_SAF_D = 434
    CEFBS_HasBasicF, // FCMP_SAF_S = 435
    CEFBS_HasBasicD, // FCMP_SEQ_D = 436
    CEFBS_HasBasicF, // FCMP_SEQ_S = 437
    CEFBS_HasBasicD, // FCMP_SLE_D = 438
    CEFBS_HasBasicF, // FCMP_SLE_S = 439
    CEFBS_HasBasicD, // FCMP_SLT_D = 440
    CEFBS_HasBasicF, // FCMP_SLT_S = 441
    CEFBS_HasBasicD, // FCMP_SNE_D = 442
    CEFBS_HasBasicF, // FCMP_SNE_S = 443
    CEFBS_HasBasicD, // FCMP_SOR_D = 444
    CEFBS_HasBasicF, // FCMP_SOR_S = 445
    CEFBS_HasBasicD, // FCMP_SUEQ_D = 446
    CEFBS_HasBasicF, // FCMP_SUEQ_S = 447
    CEFBS_HasBasicD, // FCMP_SULE_D = 448
    CEFBS_HasBasicF, // FCMP_SULE_S = 449
    CEFBS_HasBasicD, // FCMP_SULT_D = 450
    CEFBS_HasBasicF, // FCMP_SULT_S = 451
    CEFBS_HasBasicD, // FCMP_SUNE_D = 452
    CEFBS_HasBasicF, // FCMP_SUNE_S = 453
    CEFBS_HasBasicD, // FCMP_SUN_D = 454
    CEFBS_HasBasicF, // FCMP_SUN_S = 455
    CEFBS_HasBasicD, // FCOPYSIGN_D = 456
    CEFBS_HasBasicF, // FCOPYSIGN_S = 457
    CEFBS_HasBasicD, // FCVT_D_S = 458
    CEFBS_HasBasicD, // FCVT_S_D = 459
    CEFBS_HasBasicD, // FDIV_D = 460
    CEFBS_HasBasicF, // FDIV_S = 461
    CEFBS_HasBasicD, // FFINT_D_L = 462
    CEFBS_HasBasicD, // FFINT_D_W = 463
    CEFBS_HasBasicD, // FFINT_S_L = 464
    CEFBS_HasBasicF, // FFINT_S_W = 465
    CEFBS_HasBasicD, // FLDGT_D = 466
    CEFBS_HasBasicF, // FLDGT_S = 467
    CEFBS_HasBasicD, // FLDLE_D = 468
    CEFBS_HasBasicF, // FLDLE_S = 469
    CEFBS_HasBasicD, // FLDX_D = 470
    CEFBS_HasBasicF, // FLDX_S = 471
    CEFBS_HasBasicD, // FLD_D = 472
    CEFBS_HasBasicF, // FLD_S = 473
    CEFBS_HasBasicD, // FLOGB_D = 474
    CEFBS_HasBasicF, // FLOGB_S = 475
    CEFBS_HasBasicD, // FMADD_D = 476
    CEFBS_HasBasicF, // FMADD_S = 477
    CEFBS_HasBasicD, // FMAXA_D = 478
    CEFBS_HasBasicF, // FMAXA_S = 479
    CEFBS_HasBasicD, // FMAX_D = 480
    CEFBS_HasBasicF, // FMAX_S = 481
    CEFBS_HasBasicD, // FMINA_D = 482
    CEFBS_HasBasicF, // FMINA_S = 483
    CEFBS_HasBasicD, // FMIN_D = 484
    CEFBS_HasBasicF, // FMIN_S = 485
    CEFBS_HasBasicD, // FMOV_D = 486
    CEFBS_HasBasicF, // FMOV_S = 487
    CEFBS_HasBasicD, // FMSUB_D = 488
    CEFBS_HasBasicF, // FMSUB_S = 489
    CEFBS_HasBasicD, // FMUL_D = 490
    CEFBS_HasBasicF, // FMUL_S = 491
    CEFBS_HasBasicD, // FNEG_D = 492
    CEFBS_HasBasicF, // FNEG_S = 493
    CEFBS_HasBasicD, // FNMADD_D = 494
    CEFBS_HasBasicF, // FNMADD_S = 495
    CEFBS_HasBasicD, // FNMSUB_D = 496
    CEFBS_HasBasicF, // FNMSUB_S = 497
    CEFBS_HasBasicD, // FRECIP_D = 498
    CEFBS_HasBasicF, // FRECIP_S = 499
    CEFBS_HasBasicD, // FRINT_D = 500
    CEFBS_HasBasicF, // FRINT_S = 501
    CEFBS_HasBasicD, // FRSQRT_D = 502
    CEFBS_HasBasicF, // FRSQRT_S = 503
    CEFBS_HasBasicD, // FSCALEB_D = 504
    CEFBS_HasBasicF, // FSCALEB_S = 505
    CEFBS_HasBasicD, // FSEL_D = 506
    CEFBS_HasBasicF, // FSEL_S = 507
    CEFBS_HasBasicD, // FSQRT_D = 508
    CEFBS_HasBasicF, // FSQRT_S = 509
    CEFBS_HasBasicD, // FSTGT_D = 510
    CEFBS_HasBasicF, // FSTGT_S = 511
    CEFBS_HasBasicD, // FSTLE_D = 512
    CEFBS_HasBasicF, // FSTLE_S = 513
    CEFBS_HasBasicD, // FSTX_D = 514
    CEFBS_HasBasicF, // FSTX_S = 515
    CEFBS_HasBasicD, // FST_D = 516
    CEFBS_HasBasicF, // FST_S = 517
    CEFBS_HasBasicD, // FSUB_D = 518
    CEFBS_HasBasicF, // FSUB_S = 519
    CEFBS_HasBasicD, // FTINTRM_L_D = 520
    CEFBS_HasBasicD, // FTINTRM_L_S = 521
    CEFBS_HasBasicD, // FTINTRM_W_D = 522
    CEFBS_HasBasicF, // FTINTRM_W_S = 523
    CEFBS_HasBasicD, // FTINTRNE_L_D = 524
    CEFBS_HasBasicD, // FTINTRNE_L_S = 525
    CEFBS_HasBasicD, // FTINTRNE_W_D = 526
    CEFBS_HasBasicF, // FTINTRNE_W_S = 527
    CEFBS_HasBasicD, // FTINTRP_L_D = 528
    CEFBS_HasBasicD, // FTINTRP_L_S = 529
    CEFBS_HasBasicD, // FTINTRP_W_D = 530
    CEFBS_HasBasicF, // FTINTRP_W_S = 531
    CEFBS_HasBasicD, // FTINTRZ_L_D = 532
    CEFBS_HasBasicD, // FTINTRZ_L_S = 533
    CEFBS_HasBasicD, // FTINTRZ_W_D = 534
    CEFBS_HasBasicF, // FTINTRZ_W_S = 535
    CEFBS_HasBasicD, // FTINT_L_D = 536
    CEFBS_HasBasicD, // FTINT_L_S = 537
    CEFBS_HasBasicD, // FTINT_W_D = 538
    CEFBS_HasBasicF, // FTINT_W_S = 539
    CEFBS_None, // IBAR = 540
    CEFBS_None, // IDLE = 541
    CEFBS_None, // INVTLB = 542
    CEFBS_None, // IOCSRRD_B = 543
    CEFBS_IsLA64, // IOCSRRD_D = 544
    CEFBS_None, // IOCSRRD_H = 545
    CEFBS_None, // IOCSRRD_W = 546
    CEFBS_None, // IOCSRWR_B = 547
    CEFBS_IsLA64, // IOCSRWR_D = 548
    CEFBS_None, // IOCSRWR_H = 549
    CEFBS_None, // IOCSRWR_W = 550
    CEFBS_None, // JIRL = 551
    CEFBS_None, // LDDIR = 552
    CEFBS_IsLA64, // LDGT_B = 553
    CEFBS_IsLA64, // LDGT_D = 554
    CEFBS_IsLA64, // LDGT_H = 555
    CEFBS_IsLA64, // LDGT_W = 556
    CEFBS_IsLA64, // LDLE_B = 557
    CEFBS_IsLA64, // LDLE_D = 558
    CEFBS_IsLA64, // LDLE_H = 559
    CEFBS_IsLA64, // LDLE_W = 560
    CEFBS_None, // LDPTE = 561
    CEFBS_IsLA64, // LDPTR_D = 562
    CEFBS_IsLA64, // LDPTR_W = 563
    CEFBS_IsLA64, // LDX_B = 564
    CEFBS_IsLA64, // LDX_BU = 565
    CEFBS_IsLA64, // LDX_D = 566
    CEFBS_IsLA64, // LDX_H = 567
    CEFBS_IsLA64, // LDX_HU = 568
    CEFBS_IsLA64, // LDX_W = 569
    CEFBS_IsLA64, // LDX_WU = 570
    CEFBS_None, // LD_B = 571
    CEFBS_None, // LD_BU = 572
    CEFBS_IsLA64, // LD_D = 573
    CEFBS_None, // LD_H = 574
    CEFBS_None, // LD_HU = 575
    CEFBS_None, // LD_W = 576
    CEFBS_IsLA64, // LD_WU = 577
    CEFBS_IsLA64, // LL_D = 578
    CEFBS_None, // LL_W = 579
    CEFBS_None, // LU12I_W = 580
    CEFBS_IsLA64, // LU32I_D = 581
    CEFBS_IsLA64, // LU52I_D = 582
    CEFBS_None, // MASKEQZ = 583
    CEFBS_None, // MASKNEZ = 584
    CEFBS_IsLA64, // MOD_D = 585
    CEFBS_IsLA64, // MOD_DU = 586
    CEFBS_None, // MOD_W = 587
    CEFBS_None, // MOD_WU = 588
    CEFBS_HasBasicF, // MOVCF2FR_S = 589
    CEFBS_HasBasicF, // MOVCF2GR = 590
    CEFBS_HasBasicF, // MOVFCSR2GR = 591
    CEFBS_HasBasicF, // MOVFR2CF_S = 592
    CEFBS_HasBasicD_IsLA64, // MOVFR2GR_D = 593
    CEFBS_HasBasicF, // MOVFR2GR_S = 594
    CEFBS_HasBasicD, // MOVFR2GR_S_64 = 595
    CEFBS_HasBasicD, // MOVFRH2GR_S = 596
    CEFBS_HasBasicF, // MOVGR2CF = 597
    CEFBS_HasBasicF, // MOVGR2FCSR = 598
    CEFBS_HasBasicD, // MOVGR2FRH_W = 599
    CEFBS_HasBasicD_IsLA64, // MOVGR2FR_D = 600
    CEFBS_HasBasicF, // MOVGR2FR_W = 601
    CEFBS_HasBasicD_IsLA32, // MOVGR2FR_W_64 = 602
    CEFBS_IsLA64, // MULH_D = 603
    CEFBS_IsLA64, // MULH_DU = 604
    CEFBS_None, // MULH_W = 605
    CEFBS_None, // MULH_WU = 606
    CEFBS_IsLA64, // MULW_D_W = 607
    CEFBS_IsLA64, // MULW_D_WU = 608
    CEFBS_IsLA64, // MUL_D = 609
    CEFBS_None, // MUL_W = 610
    CEFBS_None, // NOR = 611
    CEFBS_None, // OR = 612
    CEFBS_None, // ORI = 613
    CEFBS_None, // ORN = 614
    CEFBS_None, // PCADDI = 615
    CEFBS_None, // PCADDU12I = 616
    CEFBS_IsLA64, // PCADDU18I = 617
    CEFBS_None, // PCALAU12I = 618
    CEFBS_None, // PRELD = 619
    CEFBS_IsLA64, // PRELDX = 620
    CEFBS_None, // RDTIMEH_W = 621
    CEFBS_None, // RDTIMEL_W = 622
    CEFBS_IsLA64, // RDTIME_D = 623
    CEFBS_None, // REVB_2H = 624
    CEFBS_IsLA64, // REVB_2W = 625
    CEFBS_IsLA64, // REVB_4H = 626
    CEFBS_IsLA64, // REVB_D = 627
    CEFBS_IsLA64, // REVH_2W = 628
    CEFBS_IsLA64, // REVH_D = 629
    CEFBS_IsLA64, // ROTRI_D = 630
    CEFBS_None, // ROTRI_W = 631
    CEFBS_IsLA64, // ROTR_D = 632
    CEFBS_None, // ROTR_W = 633
    CEFBS_IsLA64, // SC_D = 634
    CEFBS_None, // SC_W = 635
    CEFBS_IsLA64, // SLLI_D = 636
    CEFBS_None, // SLLI_W = 637
    CEFBS_IsLA64, // SLL_D = 638
    CEFBS_None, // SLL_W = 639
    CEFBS_None, // SLT = 640
    CEFBS_None, // SLTI = 641
    CEFBS_None, // SLTU = 642
    CEFBS_None, // SLTUI = 643
    CEFBS_IsLA64, // SRAI_D = 644
    CEFBS_None, // SRAI_W = 645
    CEFBS_IsLA64, // SRA_D = 646
    CEFBS_None, // SRA_W = 647
    CEFBS_IsLA64, // SRLI_D = 648
    CEFBS_None, // SRLI_W = 649
    CEFBS_IsLA64, // SRL_D = 650
    CEFBS_None, // SRL_W = 651
    CEFBS_IsLA64, // STGT_B = 652
    CEFBS_IsLA64, // STGT_D = 653
    CEFBS_IsLA64, // STGT_H = 654
    CEFBS_IsLA64, // STGT_W = 655
    CEFBS_IsLA64, // STLE_B = 656
    CEFBS_IsLA64, // STLE_D = 657
    CEFBS_IsLA64, // STLE_H = 658
    CEFBS_IsLA64, // STLE_W = 659
    CEFBS_IsLA64, // STPTR_D = 660
    CEFBS_IsLA64, // STPTR_W = 661
    CEFBS_IsLA64, // STX_B = 662
    CEFBS_IsLA64, // STX_D = 663
    CEFBS_IsLA64, // STX_H = 664
    CEFBS_IsLA64, // STX_W = 665
    CEFBS_None, // ST_B = 666
    CEFBS_IsLA64, // ST_D = 667
    CEFBS_None, // ST_H = 668
    CEFBS_None, // ST_W = 669
    CEFBS_IsLA64, // SUB_D = 670
    CEFBS_None, // SUB_W = 671
    CEFBS_None, // SYSCALL = 672
    CEFBS_None, // TLBCLR = 673
    CEFBS_None, // TLBFILL = 674
    CEFBS_None, // TLBFLUSH = 675
    CEFBS_None, // TLBRD = 676
    CEFBS_None, // TLBSRCH = 677
    CEFBS_None, // TLBWR = 678
    CEFBS_None, // XOR = 679
    CEFBS_None, // XORI = 680
  };

  assert(Opcode < 681);
  FeatureBitset AvailableFeatures = computeAvailableFeatures(Features);
  const FeatureBitset &RequiredFeatures = FeatureBitsets[RequiredFeaturesRefs[Opcode]];
  FeatureBitset MissingFeatures =
      (AvailableFeatures & RequiredFeatures) ^
      RequiredFeatures;
  if (MissingFeatures.any()) {
    std::ostringstream Msg;
    Msg << "Attempting to emit " << &LoongArchInstrNameData[LoongArchInstrNameIndices[Opcode]]
        << " instruction but the ";
    for (unsigned i = 0, e = MissingFeatures.size(); i != e; ++i)
      if (MissingFeatures.test(i))
        Msg << SubtargetFeatureNames[i] << " ";
    Msg << "predicate(s) are not met";
    report_fatal_error(Msg.str().c_str());
  }
#endif // NDEBUG
}
} // end namespace LoongArch_MC
} // end namespace llvm
#endif // ENABLE_INSTR_PREDICATE_VERIFIER

