/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Machine Code Emitter                                                       *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

uint64_t LoongArchMCCodeEmitter::getBinaryCodeForInstr(const MCInst &MI,
    SmallVectorImpl<MCFixup> &Fixups,
    const MCSubtargetInfo &STI) const {
  static const uint64_t InstBits[] = {
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(0),
    UINT64_C(46137344),	// ADDI_D
    UINT64_C(41943040),	// ADDI_W
    UINT64_C(268435456),	// ADDU16I_D
    UINT64_C(1081344),	// ADD_D
    UINT64_C(1048576),	// ADD_W
    UINT64_C(2883584),	// ALSL_D
    UINT64_C(262144),	// ALSL_W
    UINT64_C(393216),	// ALSL_WU
    UINT64_C(945913856),	// AMADD_D
    UINT64_C(946503680),	// AMADD_DB_D
    UINT64_C(946470912),	// AMADD_DB_W
    UINT64_C(945881088),	// AMADD_W
    UINT64_C(945979392),	// AMAND_D
    UINT64_C(946569216),	// AMAND_DB_D
    UINT64_C(946536448),	// AMAND_DB_W
    UINT64_C(945946624),	// AMAND_W
    UINT64_C(946176000),	// AMMAX_D
    UINT64_C(946765824),	// AMMAX_DB_D
    UINT64_C(946896896),	// AMMAX_DB_DU
    UINT64_C(946733056),	// AMMAX_DB_W
    UINT64_C(946864128),	// AMMAX_DB_WU
    UINT64_C(946307072),	// AMMAX_DU
    UINT64_C(946143232),	// AMMAX_W
    UINT64_C(946274304),	// AMMAX_WU
    UINT64_C(946241536),	// AMMIN_D
    UINT64_C(946831360),	// AMMIN_DB_D
    UINT64_C(946962432),	// AMMIN_DB_DU
    UINT64_C(946798592),	// AMMIN_DB_W
    UINT64_C(946929664),	// AMMIN_DB_WU
    UINT64_C(946372608),	// AMMIN_DU
    UINT64_C(946208768),	// AMMIN_W
    UINT64_C(946339840),	// AMMIN_WU
    UINT64_C(946044928),	// AMOR_D
    UINT64_C(946634752),	// AMOR_DB_D
    UINT64_C(946601984),	// AMOR_DB_W
    UINT64_C(946012160),	// AMOR_W
    UINT64_C(945848320),	// AMSWAP_D
    UINT64_C(946438144),	// AMSWAP_DB_D
    UINT64_C(946405376),	// AMSWAP_DB_W
    UINT64_C(945815552),	// AMSWAP_W
    UINT64_C(946110464),	// AMXOR_D
    UINT64_C(946700288),	// AMXOR_DB_D
    UINT64_C(946667520),	// AMXOR_DB_W
    UINT64_C(946077696),	// AMXOR_W
    UINT64_C(1343488),	// AND
    UINT64_C(54525952),	// ANDI
    UINT64_C(1474560),	// ANDN
    UINT64_C(98304),	// ASRTGT_D
    UINT64_C(65536),	// ASRTLE_D
    UINT64_C(1342177280),	// B
    UINT64_C(1207959552),	// BCEQZ
    UINT64_C(1207959808),	// BCNEZ
    UINT64_C(1476395008),	// BEQ
    UINT64_C(1073741824),	// BEQZ
    UINT64_C(1677721600),	// BGE
    UINT64_C(1811939328),	// BGEU
    UINT64_C(18432),	// BITREV_4B
    UINT64_C(19456),	// BITREV_8B
    UINT64_C(21504),	// BITREV_D
    UINT64_C(20480),	// BITREV_W
    UINT64_C(1409286144),	// BL
    UINT64_C(1610612736),	// BLT
    UINT64_C(1744830464),	// BLTU
    UINT64_C(1543503872),	// BNE
    UINT64_C(1140850688),	// BNEZ
    UINT64_C(2752512),	// BREAK
    UINT64_C(8388608),	// BSTRINS_D
    UINT64_C(6291456),	// BSTRINS_W
    UINT64_C(12582912),	// BSTRPICK_D
    UINT64_C(6324224),	// BSTRPICK_W
    UINT64_C(786432),	// BYTEPICK_D
    UINT64_C(524288),	// BYTEPICK_W
    UINT64_C(100663296),	// CACOP
    UINT64_C(8192),	// CLO_D
    UINT64_C(4096),	// CLO_W
    UINT64_C(9216),	// CLZ_D
    UINT64_C(5120),	// CLZ_W
    UINT64_C(27648),	// CPUCFG
    UINT64_C(2490368),	// CRCC_W_B_W
    UINT64_C(2588672),	// CRCC_W_D_W
    UINT64_C(2523136),	// CRCC_W_H_W
    UINT64_C(2555904),	// CRCC_W_W_W
    UINT64_C(2359296),	// CRC_W_B_W
    UINT64_C(2457600),	// CRC_W_D_W
    UINT64_C(2392064),	// CRC_W_H_W
    UINT64_C(2424832),	// CRC_W_W_W
    UINT64_C(67108864),	// CSRRD
    UINT64_C(67108896),	// CSRWR
    UINT64_C(67108864),	// CSRXCHG
    UINT64_C(10240),	// CTO_D
    UINT64_C(6144),	// CTO_W
    UINT64_C(11264),	// CTZ_D
    UINT64_C(7168),	// CTZ_W
    UINT64_C(946995200),	// DBAR
    UINT64_C(2785280),	// DBCL
    UINT64_C(2228224),	// DIV_D
    UINT64_C(2293760),	// DIV_DU
    UINT64_C(2097152),	// DIV_W
    UINT64_C(2162688),	// DIV_WU
    UINT64_C(105396224),	// ERTN
    UINT64_C(23552),	// EXT_W_B
    UINT64_C(22528),	// EXT_W_H
    UINT64_C(18089984),	// FABS_D
    UINT64_C(18088960),	// FABS_S
    UINT64_C(16842752),	// FADD_D
    UINT64_C(16809984),	// FADD_S
    UINT64_C(18102272),	// FCLASS_D
    UINT64_C(18101248),	// FCLASS_S
    UINT64_C(203423744),	// FCMP_CAF_D
    UINT64_C(202375168),	// FCMP_CAF_S
    UINT64_C(203554816),	// FCMP_CEQ_D
    UINT64_C(202506240),	// FCMP_CEQ_S
    UINT64_C(203620352),	// FCMP_CLE_D
    UINT64_C(202571776),	// FCMP_CLE_S
    UINT64_C(203489280),	// FCMP_CLT_D
    UINT64_C(202440704),	// FCMP_CLT_S
    UINT64_C(203948032),	// FCMP_CNE_D
    UINT64_C(202899456),	// FCMP_CNE_S
    UINT64_C(204079104),	// FCMP_COR_D
    UINT64_C(203030528),	// FCMP_COR_S
    UINT64_C(203816960),	// FCMP_CUEQ_D
    UINT64_C(202768384),	// FCMP_CUEQ_S
    UINT64_C(203882496),	// FCMP_CULE_D
    UINT64_C(202833920),	// FCMP_CULE_S
    UINT64_C(203751424),	// FCMP_CULT_D
    UINT64_C(202702848),	// FCMP_CULT_S
    UINT64_C(204210176),	// FCMP_CUNE_D
    UINT64_C(203161600),	// FCMP_CUNE_S
    UINT64_C(203685888),	// FCMP_CUN_D
    UINT64_C(202637312),	// FCMP_CUN_S
    UINT64_C(203456512),	// FCMP_SAF_D
    UINT64_C(202407936),	// FCMP_SAF_S
    UINT64_C(203587584),	// FCMP_SEQ_D
    UINT64_C(202539008),	// FCMP_SEQ_S
    UINT64_C(203653120),	// FCMP_SLE_D
    UINT64_C(202604544),	// FCMP_SLE_S
    UINT64_C(203522048),	// FCMP_SLT_D
    UINT64_C(202473472),	// FCMP_SLT_S
    UINT64_C(203980800),	// FCMP_SNE_D
    UINT64_C(202932224),	// FCMP_SNE_S
    UINT64_C(204111872),	// FCMP_SOR_D
    UINT64_C(203063296),	// FCMP_SOR_S
    UINT64_C(203849728),	// FCMP_SUEQ_D
    UINT64_C(202801152),	// FCMP_SUEQ_S
    UINT64_C(203915264),	// FCMP_SULE_D
    UINT64_C(202866688),	// FCMP_SULE_S
    UINT64_C(203784192),	// FCMP_SULT_D
    UINT64_C(202735616),	// FCMP_SULT_S
    UINT64_C(204242944),	// FCMP_SUNE_D
    UINT64_C(203194368),	// FCMP_SUNE_S
    UINT64_C(203718656),	// FCMP_SUN_D
    UINT64_C(202670080),	// FCMP_SUN_S
    UINT64_C(18022400),	// FCOPYSIGN_D
    UINT64_C(17989632),	// FCOPYSIGN_S
    UINT64_C(18424832),	// FCVT_D_S
    UINT64_C(18421760),	// FCVT_S_D
    UINT64_C(17235968),	// FDIV_D
    UINT64_C(17203200),	// FDIV_S
    UINT64_C(18688000),	// FFINT_D_L
    UINT64_C(18685952),	// FFINT_D_W
    UINT64_C(18683904),	// FFINT_S_L
    UINT64_C(18681856),	// FFINT_S_W
    UINT64_C(947159040),	// FLDGT_D
    UINT64_C(947126272),	// FLDGT_S
    UINT64_C(947224576),	// FLDLE_D
    UINT64_C(947191808),	// FLDLE_S
    UINT64_C(942931968),	// FLDX_D
    UINT64_C(942669824),	// FLDX_S
    UINT64_C(729808896),	// FLD_D
    UINT64_C(721420288),	// FLD_S
    UINT64_C(18098176),	// FLOGB_D
    UINT64_C(18097152),	// FLOGB_S
    UINT64_C(136314880),	// FMADD_D
    UINT64_C(135266304),	// FMADD_S
    UINT64_C(17629184),	// FMAXA_D
    UINT64_C(17596416),	// FMAXA_S
    UINT64_C(17367040),	// FMAX_D
    UINT64_C(17334272),	// FMAX_S
    UINT64_C(17760256),	// FMINA_D
    UINT64_C(17727488),	// FMINA_S
    UINT64_C(17498112),	// FMIN_D
    UINT64_C(17465344),	// FMIN_S
    UINT64_C(18126848),	// FMOV_D
    UINT64_C(18125824),	// FMOV_S
    UINT64_C(140509184),	// FMSUB_D
    UINT64_C(139460608),	// FMSUB_S
    UINT64_C(17104896),	// FMUL_D
    UINT64_C(17072128),	// FMUL_S
    UINT64_C(18094080),	// FNEG_D
    UINT64_C(18093056),	// FNEG_S
    UINT64_C(144703488),	// FNMADD_D
    UINT64_C(143654912),	// FNMADD_S
    UINT64_C(148897792),	// FNMSUB_D
    UINT64_C(147849216),	// FNMSUB_S
    UINT64_C(18110464),	// FRECIP_D
    UINT64_C(18109440),	// FRECIP_S
    UINT64_C(18761728),	// FRINT_D
    UINT64_C(18760704),	// FRINT_S
    UINT64_C(18114560),	// FRSQRT_D
    UINT64_C(18113536),	// FRSQRT_S
    UINT64_C(17891328),	// FSCALEB_D
    UINT64_C(17858560),	// FSCALEB_S
    UINT64_C(218103808),	// FSEL_D
    UINT64_C(218103808),	// FSEL_S
    UINT64_C(18106368),	// FSQRT_D
    UINT64_C(18105344),	// FSQRT_S
    UINT64_C(947290112),	// FSTGT_D
    UINT64_C(947257344),	// FSTGT_S
    UINT64_C(947355648),	// FSTLE_D
    UINT64_C(947322880),	// FSTLE_S
    UINT64_C(943456256),	// FSTX_D
    UINT64_C(943194112),	// FSTX_S
    UINT64_C(734003200),	// FST_D
    UINT64_C(725614592),	// FST_S
    UINT64_C(16973824),	// FSUB_D
    UINT64_C(16941056),	// FSUB_S
    UINT64_C(18491392),	// FTINTRM_L_D
    UINT64_C(18490368),	// FTINTRM_L_S
    UINT64_C(18483200),	// FTINTRM_W_D
    UINT64_C(18482176),	// FTINTRM_W_S
    UINT64_C(18540544),	// FTINTRNE_L_D
    UINT64_C(18539520),	// FTINTRNE_L_S
    UINT64_C(18532352),	// FTINTRNE_W_D
    UINT64_C(18531328),	// FTINTRNE_W_S
    UINT64_C(18507776),	// FTINTRP_L_D
    UINT64_C(18506752),	// FTINTRP_L_S
    UINT64_C(18499584),	// FTINTRP_W_D
    UINT64_C(18498560),	// FTINTRP_W_S
    UINT64_C(18524160),	// FTINTRZ_L_D
    UINT64_C(18523136),	// FTINTRZ_L_S
    UINT64_C(18515968),	// FTINTRZ_W_D
    UINT64_C(18514944),	// FTINTRZ_W_S
    UINT64_C(18556928),	// FTINT_L_D
    UINT64_C(18555904),	// FTINT_L_S
    UINT64_C(18548736),	// FTINT_W_D
    UINT64_C(18547712),	// FTINT_W_S
    UINT64_C(947027968),	// IBAR
    UINT64_C(105414656),	// IDLE
    UINT64_C(105480192),	// INVTLB
    UINT64_C(105381888),	// IOCSRRD_B
    UINT64_C(105384960),	// IOCSRRD_D
    UINT64_C(105382912),	// IOCSRRD_H
    UINT64_C(105383936),	// IOCSRRD_W
    UINT64_C(105385984),	// IOCSRWR_B
    UINT64_C(105389056),	// IOCSRWR_D
    UINT64_C(105387008),	// IOCSRWR_H
    UINT64_C(105388032),	// IOCSRWR_W
    UINT64_C(1275068416),	// JIRL
    UINT64_C(104857600),	// LDDIR
    UINT64_C(947388416),	// LDGT_B
    UINT64_C(947486720),	// LDGT_D
    UINT64_C(947421184),	// LDGT_H
    UINT64_C(947453952),	// LDGT_W
    UINT64_C(947519488),	// LDLE_B
    UINT64_C(947617792),	// LDLE_D
    UINT64_C(947552256),	// LDLE_H
    UINT64_C(947585024),	// LDLE_W
    UINT64_C(105119744),	// LDPTE
    UINT64_C(637534208),	// LDPTR_D
    UINT64_C(603979776),	// LDPTR_W
    UINT64_C(939524096),	// LDX_B
    UINT64_C(941621248),	// LDX_BU
    UINT64_C(940310528),	// LDX_D
    UINT64_C(939786240),	// LDX_H
    UINT64_C(941883392),	// LDX_HU
    UINT64_C(940048384),	// LDX_W
    UINT64_C(942145536),	// LDX_WU
    UINT64_C(671088640),	// LD_B
    UINT64_C(704643072),	// LD_BU
    UINT64_C(683671552),	// LD_D
    UINT64_C(675282944),	// LD_H
    UINT64_C(708837376),	// LD_HU
    UINT64_C(679477248),	// LD_W
    UINT64_C(713031680),	// LD_WU
    UINT64_C(570425344),	// LL_D
    UINT64_C(536870912),	// LL_W
    UINT64_C(335544320),	// LU12I_W
    UINT64_C(369098752),	// LU32I_D
    UINT64_C(50331648),	// LU52I_D
    UINT64_C(1245184),	// MASKEQZ
    UINT64_C(1277952),	// MASKNEZ
    UINT64_C(2260992),	// MOD_D
    UINT64_C(2326528),	// MOD_DU
    UINT64_C(2129920),	// MOD_W
    UINT64_C(2195456),	// MOD_WU
    UINT64_C(18142208),	// MOVCF2FR_S
    UINT64_C(18144256),	// MOVCF2GR
    UINT64_C(18139136),	// MOVFCSR2GR
    UINT64_C(18141184),	// MOVFR2CF_S
    UINT64_C(18135040),	// MOVFR2GR_D
    UINT64_C(18134016),	// MOVFR2GR_S
    UINT64_C(18134016),	// MOVFR2GR_S_64
    UINT64_C(18136064),	// MOVFRH2GR_S
    UINT64_C(18143232),	// MOVGR2CF
    UINT64_C(18137088),	// MOVGR2FCSR
    UINT64_C(18131968),	// MOVGR2FRH_W
    UINT64_C(18130944),	// MOVGR2FR_D
    UINT64_C(18129920),	// MOVGR2FR_W
    UINT64_C(18129920),	// MOVGR2FR_W_64
    UINT64_C(1966080),	// MULH_D
    UINT64_C(1998848),	// MULH_DU
    UINT64_C(1867776),	// MULH_W
    UINT64_C(1900544),	// MULH_WU
    UINT64_C(2031616),	// MULW_D_W
    UINT64_C(2064384),	// MULW_D_WU
    UINT64_C(1933312),	// MUL_D
    UINT64_C(1835008),	// MUL_W
    UINT64_C(1310720),	// NOR
    UINT64_C(1376256),	// OR
    UINT64_C(58720256),	// ORI
    UINT64_C(1441792),	// ORN
    UINT64_C(402653184),	// PCADDI
    UINT64_C(469762048),	// PCADDU12I
    UINT64_C(503316480),	// PCADDU18I
    UINT64_C(436207616),	// PCALAU12I
    UINT64_C(717225984),	// PRELD
    UINT64_C(942407680),	// PRELDX
    UINT64_C(25600),	// RDTIMEH_W
    UINT64_C(24576),	// RDTIMEL_W
    UINT64_C(26624),	// RDTIME_D
    UINT64_C(12288),	// REVB_2H
    UINT64_C(14336),	// REVB_2W
    UINT64_C(13312),	// REVB_4H
    UINT64_C(15360),	// REVB_D
    UINT64_C(16384),	// REVH_2W
    UINT64_C(17408),	// REVH_D
    UINT64_C(5046272),	// ROTRI_D
    UINT64_C(5013504),	// ROTRI_W
    UINT64_C(1802240),	// ROTR_D
    UINT64_C(1769472),	// ROTR_W
    UINT64_C(587202560),	// SC_D
    UINT64_C(553648128),	// SC_W
    UINT64_C(4259840),	// SLLI_D
    UINT64_C(4227072),	// SLLI_W
    UINT64_C(1605632),	// SLL_D
    UINT64_C(1507328),	// SLL_W
    UINT64_C(1179648),	// SLT
    UINT64_C(33554432),	// SLTI
    UINT64_C(1212416),	// SLTU
    UINT64_C(37748736),	// SLTUI
    UINT64_C(4784128),	// SRAI_D
    UINT64_C(4751360),	// SRAI_W
    UINT64_C(1671168),	// SRA_D
    UINT64_C(1572864),	// SRA_W
    UINT64_C(4521984),	// SRLI_D
    UINT64_C(4489216),	// SRLI_W
    UINT64_C(1638400),	// SRL_D
    UINT64_C(1540096),	// SRL_W
    UINT64_C(947650560),	// STGT_B
    UINT64_C(947748864),	// STGT_D
    UINT64_C(947683328),	// STGT_H
    UINT64_C(947716096),	// STGT_W
    UINT64_C(947781632),	// STLE_B
    UINT64_C(947879936),	// STLE_D
    UINT64_C(947814400),	// STLE_H
    UINT64_C(947847168),	// STLE_W
    UINT64_C(654311424),	// STPTR_D
    UINT64_C(620756992),	// STPTR_W
    UINT64_C(940572672),	// STX_B
    UINT64_C(941359104),	// STX_D
    UINT64_C(940834816),	// STX_H
    UINT64_C(941096960),	// STX_W
    UINT64_C(687865856),	// ST_B
    UINT64_C(700448768),	// ST_D
    UINT64_C(692060160),	// ST_H
    UINT64_C(696254464),	// ST_W
    UINT64_C(1146880),	// SUB_D
    UINT64_C(1114112),	// SUB_W
    UINT64_C(2818048),	// SYSCALL
    UINT64_C(105390080),	// TLBCLR
    UINT64_C(105395200),	// TLBFILL
    UINT64_C(105391104),	// TLBFLUSH
    UINT64_C(105393152),	// TLBRD
    UINT64_C(105392128),	// TLBSRCH
    UINT64_C(105394176),	// TLBWR
    UINT64_C(1409024),	// XOR
    UINT64_C(62914560),	// XORI
    UINT64_C(0)
  };
  const unsigned opcode = MI.getOpcode();
  uint64_t Value = InstBits[opcode];
  uint64_t op = 0;
  (void)op;  // suppress warning
  switch (opcode) {
    case LoongArch::ERTN:
    case LoongArch::TLBCLR:
    case LoongArch::TLBFILL:
    case LoongArch::TLBFLUSH:
    case LoongArch::TLBRD:
    case LoongArch::TLBSRCH:
    case LoongArch::TLBWR: {
      break;
    }
    case LoongArch::FSEL_D:
    case LoongArch::FSEL_S: {
      // op: ca
      op = getMachineOpValue(MI, MI.getOperand(3), Fixups, STI);
      op &= UINT64_C(7);
      op <<= 15;
      Value |= op;
      // op: fk
      op = getMachineOpValue(MI, MI.getOperand(2), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 10;
      Value |= op;
      // op: fj
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: fd
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::CSRRD: {
      // op: csr_num
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(16383);
      op <<= 10;
      Value |= op;
      // op: rd
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::CSRWR: {
      // op: csr_num
      op = getMachineOpValue(MI, MI.getOperand(2), Fixups, STI);
      op &= UINT64_C(16383);
      op <<= 10;
      Value |= op;
      // op: rd
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::CSRXCHG: {
      // op: csr_num
      op = getMachineOpValue(MI, MI.getOperand(3), Fixups, STI);
      op &= UINT64_C(16383);
      op <<= 10;
      Value |= op;
      // op: rj
      op = getMachineOpValue(MI, MI.getOperand(2), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: rd
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::FMADD_D:
    case LoongArch::FMADD_S:
    case LoongArch::FMSUB_D:
    case LoongArch::FMSUB_S:
    case LoongArch::FNMADD_D:
    case LoongArch::FNMADD_S:
    case LoongArch::FNMSUB_D:
    case LoongArch::FNMSUB_S: {
      // op: fa
      op = getMachineOpValue(MI, MI.getOperand(3), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 15;
      Value |= op;
      // op: fk
      op = getMachineOpValue(MI, MI.getOperand(2), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 10;
      Value |= op;
      // op: fj
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: fd
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::FABS_D:
    case LoongArch::FABS_S:
    case LoongArch::FCLASS_D:
    case LoongArch::FCLASS_S:
    case LoongArch::FCVT_D_S:
    case LoongArch::FCVT_S_D:
    case LoongArch::FFINT_D_L:
    case LoongArch::FFINT_D_W:
    case LoongArch::FFINT_S_L:
    case LoongArch::FFINT_S_W:
    case LoongArch::FLOGB_D:
    case LoongArch::FLOGB_S:
    case LoongArch::FNEG_D:
    case LoongArch::FNEG_S:
    case LoongArch::FRECIP_D:
    case LoongArch::FRECIP_S:
    case LoongArch::FRINT_D:
    case LoongArch::FRINT_S:
    case LoongArch::FRSQRT_D:
    case LoongArch::FRSQRT_S:
    case LoongArch::FSQRT_D:
    case LoongArch::FSQRT_S:
    case LoongArch::FTINTRM_L_D:
    case LoongArch::FTINTRM_L_S:
    case LoongArch::FTINTRM_W_D:
    case LoongArch::FTINTRM_W_S:
    case LoongArch::FTINTRNE_L_D:
    case LoongArch::FTINTRNE_L_S:
    case LoongArch::FTINTRNE_W_D:
    case LoongArch::FTINTRNE_W_S:
    case LoongArch::FTINTRP_L_D:
    case LoongArch::FTINTRP_L_S:
    case LoongArch::FTINTRP_W_D:
    case LoongArch::FTINTRP_W_S:
    case LoongArch::FTINTRZ_L_D:
    case LoongArch::FTINTRZ_L_S:
    case LoongArch::FTINTRZ_W_D:
    case LoongArch::FTINTRZ_W_S:
    case LoongArch::FTINT_L_D:
    case LoongArch::FTINT_L_S:
    case LoongArch::FTINT_W_D:
    case LoongArch::FTINT_W_S: {
      // op: fj
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: fd
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::FCMP_CAF_D:
    case LoongArch::FCMP_CAF_S:
    case LoongArch::FCMP_CEQ_D:
    case LoongArch::FCMP_CEQ_S:
    case LoongArch::FCMP_CLE_D:
    case LoongArch::FCMP_CLE_S:
    case LoongArch::FCMP_CLT_D:
    case LoongArch::FCMP_CLT_S:
    case LoongArch::FCMP_CNE_D:
    case LoongArch::FCMP_CNE_S:
    case LoongArch::FCMP_COR_D:
    case LoongArch::FCMP_COR_S:
    case LoongArch::FCMP_CUEQ_D:
    case LoongArch::FCMP_CUEQ_S:
    case LoongArch::FCMP_CULE_D:
    case LoongArch::FCMP_CULE_S:
    case LoongArch::FCMP_CULT_D:
    case LoongArch::FCMP_CULT_S:
    case LoongArch::FCMP_CUNE_D:
    case LoongArch::FCMP_CUNE_S:
    case LoongArch::FCMP_CUN_D:
    case LoongArch::FCMP_CUN_S:
    case LoongArch::FCMP_SAF_D:
    case LoongArch::FCMP_SAF_S:
    case LoongArch::FCMP_SEQ_D:
    case LoongArch::FCMP_SEQ_S:
    case LoongArch::FCMP_SLE_D:
    case LoongArch::FCMP_SLE_S:
    case LoongArch::FCMP_SLT_D:
    case LoongArch::FCMP_SLT_S:
    case LoongArch::FCMP_SNE_D:
    case LoongArch::FCMP_SNE_S:
    case LoongArch::FCMP_SOR_D:
    case LoongArch::FCMP_SOR_S:
    case LoongArch::FCMP_SUEQ_D:
    case LoongArch::FCMP_SUEQ_S:
    case LoongArch::FCMP_SULE_D:
    case LoongArch::FCMP_SULE_S:
    case LoongArch::FCMP_SULT_D:
    case LoongArch::FCMP_SULT_S:
    case LoongArch::FCMP_SUNE_D:
    case LoongArch::FCMP_SUNE_S:
    case LoongArch::FCMP_SUN_D:
    case LoongArch::FCMP_SUN_S: {
      // op: fk
      op = getMachineOpValue(MI, MI.getOperand(2), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 10;
      Value |= op;
      // op: fj
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: cd
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(7);
      Value |= op;
      break;
    }
    case LoongArch::FADD_D:
    case LoongArch::FADD_S:
    case LoongArch::FCOPYSIGN_D:
    case LoongArch::FCOPYSIGN_S:
    case LoongArch::FDIV_D:
    case LoongArch::FDIV_S:
    case LoongArch::FMAXA_D:
    case LoongArch::FMAXA_S:
    case LoongArch::FMAX_D:
    case LoongArch::FMAX_S:
    case LoongArch::FMINA_D:
    case LoongArch::FMINA_S:
    case LoongArch::FMIN_D:
    case LoongArch::FMIN_S:
    case LoongArch::FMUL_D:
    case LoongArch::FMUL_S:
    case LoongArch::FSCALEB_D:
    case LoongArch::FSCALEB_S:
    case LoongArch::FSUB_D:
    case LoongArch::FSUB_S: {
      // op: fk
      op = getMachineOpValue(MI, MI.getOperand(2), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 10;
      Value |= op;
      // op: fj
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: fd
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::FLD_D:
    case LoongArch::FLD_S:
    case LoongArch::FST_D:
    case LoongArch::FST_S: {
      // op: imm12
      op = getMachineOpValue(MI, MI.getOperand(2), Fixups, STI);
      op &= UINT64_C(4095);
      op <<= 10;
      Value |= op;
      // op: rj
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: fd
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::PRELD: {
      // op: imm12
      op = getMachineOpValue(MI, MI.getOperand(2), Fixups, STI);
      op &= UINT64_C(4095);
      op <<= 10;
      Value |= op;
      // op: rj
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: imm5
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::CACOP: {
      // op: imm12
      op = getMachineOpValue(MI, MI.getOperand(2), Fixups, STI);
      op &= UINT64_C(4095);
      op <<= 10;
      Value |= op;
      // op: rj
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: op
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::ADDI_D:
    case LoongArch::ADDI_W:
    case LoongArch::ANDI:
    case LoongArch::LD_B:
    case LoongArch::LD_BU:
    case LoongArch::LD_D:
    case LoongArch::LD_H:
    case LoongArch::LD_HU:
    case LoongArch::LD_W:
    case LoongArch::LD_WU:
    case LoongArch::LU52I_D:
    case LoongArch::ORI:
    case LoongArch::SLTI:
    case LoongArch::SLTUI:
    case LoongArch::ST_B:
    case LoongArch::ST_D:
    case LoongArch::ST_H:
    case LoongArch::ST_W:
    case LoongArch::XORI: {
      // op: imm12
      op = getMachineOpValue(MI, MI.getOperand(2), Fixups, STI);
      op &= UINT64_C(4095);
      op <<= 10;
      Value |= op;
      // op: rj
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: rd
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::LDPTR_D:
    case LoongArch::LDPTR_W:
    case LoongArch::LL_D:
    case LoongArch::LL_W:
    case LoongArch::STPTR_D:
    case LoongArch::STPTR_W: {
      // op: imm14
      op = getImmOpValueAsr2(MI, 2, Fixups, STI);
      op &= UINT64_C(16383);
      op <<= 10;
      Value |= op;
      // op: rj
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: rd
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::SC_D:
    case LoongArch::SC_W: {
      // op: imm14
      op = getImmOpValueAsr2(MI, 3, Fixups, STI);
      op &= UINT64_C(16383);
      op <<= 10;
      Value |= op;
      // op: rj
      op = getMachineOpValue(MI, MI.getOperand(2), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: rd
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::BREAK:
    case LoongArch::DBAR:
    case LoongArch::DBCL:
    case LoongArch::IBAR:
    case LoongArch::IDLE:
    case LoongArch::SYSCALL: {
      // op: imm15
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(32767);
      Value |= op;
      break;
    }
    case LoongArch::BEQ:
    case LoongArch::BGE:
    case LoongArch::BGEU:
    case LoongArch::BLT:
    case LoongArch::BLTU:
    case LoongArch::BNE: {
      // op: imm16
      op = getImmOpValueAsr2(MI, 2, Fixups, STI);
      op &= UINT64_C(65535);
      op <<= 10;
      Value |= op;
      // op: rj
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: rd
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::JIRL: {
      // op: imm16
      op = getImmOpValueAsr2(MI, 2, Fixups, STI);
      op &= UINT64_C(65535);
      op <<= 10;
      Value |= op;
      // op: rj
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: rd
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::ADDU16I_D: {
      // op: imm16
      op = getMachineOpValue(MI, MI.getOperand(2), Fixups, STI);
      op &= UINT64_C(65535);
      op <<= 10;
      Value |= op;
      // op: rj
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: rd
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::ALSL_D:
    case LoongArch::ALSL_W:
    case LoongArch::ALSL_WU: {
      // op: imm2
      op = getImmOpValueSub1(MI, 3, Fixups, STI);
      op &= UINT64_C(3);
      op <<= 15;
      Value |= op;
      // op: rk
      op = getMachineOpValue(MI, MI.getOperand(2), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 10;
      Value |= op;
      // op: rj
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: rd
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::BYTEPICK_W: {
      // op: imm2
      op = getMachineOpValue(MI, MI.getOperand(3), Fixups, STI);
      op &= UINT64_C(3);
      op <<= 15;
      Value |= op;
      // op: rk
      op = getMachineOpValue(MI, MI.getOperand(2), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 10;
      Value |= op;
      // op: rj
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: rd
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::LU12I_W:
    case LoongArch::PCADDI:
    case LoongArch::PCADDU12I:
    case LoongArch::PCADDU18I:
    case LoongArch::PCALAU12I: {
      // op: imm20
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(1048575);
      op <<= 5;
      Value |= op;
      // op: rd
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::LU32I_D: {
      // op: imm20
      op = getMachineOpValue(MI, MI.getOperand(2), Fixups, STI);
      op &= UINT64_C(1048575);
      op <<= 5;
      Value |= op;
      // op: rd
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::BCEQZ:
    case LoongArch::BCNEZ: {
      // op: imm21
      op = getImmOpValueAsr2(MI, 1, Fixups, STI);
      Value |= (op & UINT64_C(65535)) << 10;
      Value |= (op & UINT64_C(2031616)) >> 16;
      // op: cj
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(7);
      op <<= 5;
      Value |= op;
      break;
    }
    case LoongArch::BEQZ:
    case LoongArch::BNEZ: {
      // op: imm21
      op = getImmOpValueAsr2(MI, 1, Fixups, STI);
      Value |= (op & UINT64_C(65535)) << 10;
      Value |= (op & UINT64_C(2031616)) >> 16;
      // op: rj
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      break;
    }
    case LoongArch::B:
    case LoongArch::BL: {
      // op: imm26
      op = getImmOpValueAsr2(MI, 0, Fixups, STI);
      Value |= (op & UINT64_C(65535)) << 10;
      Value |= (op & UINT64_C(67043328)) >> 16;
      break;
    }
    case LoongArch::BYTEPICK_D: {
      // op: imm3
      op = getMachineOpValue(MI, MI.getOperand(3), Fixups, STI);
      op &= UINT64_C(7);
      op <<= 15;
      Value |= op;
      // op: rk
      op = getMachineOpValue(MI, MI.getOperand(2), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 10;
      Value |= op;
      // op: rj
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: rd
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::ROTRI_W:
    case LoongArch::SLLI_W:
    case LoongArch::SRAI_W:
    case LoongArch::SRLI_W: {
      // op: imm5
      op = getMachineOpValue(MI, MI.getOperand(2), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 10;
      Value |= op;
      // op: rj
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: rd
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::ROTRI_D:
    case LoongArch::SLLI_D:
    case LoongArch::SRAI_D:
    case LoongArch::SRLI_D: {
      // op: imm6
      op = getMachineOpValue(MI, MI.getOperand(2), Fixups, STI);
      op &= UINT64_C(63);
      op <<= 10;
      Value |= op;
      // op: rj
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: rd
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::LDDIR: {
      // op: imm8
      op = getMachineOpValue(MI, MI.getOperand(2), Fixups, STI);
      op &= UINT64_C(255);
      op <<= 10;
      Value |= op;
      // op: rj
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: rd
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::BSTRPICK_D: {
      // op: msbd
      op = getMachineOpValue(MI, MI.getOperand(2), Fixups, STI);
      op &= UINT64_C(63);
      op <<= 16;
      Value |= op;
      // op: lsbd
      op = getMachineOpValue(MI, MI.getOperand(3), Fixups, STI);
      op &= UINT64_C(63);
      op <<= 10;
      Value |= op;
      // op: rj
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: rd
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::BSTRINS_D: {
      // op: msbd
      op = getMachineOpValue(MI, MI.getOperand(3), Fixups, STI);
      op &= UINT64_C(63);
      op <<= 16;
      Value |= op;
      // op: lsbd
      op = getMachineOpValue(MI, MI.getOperand(4), Fixups, STI);
      op &= UINT64_C(63);
      op <<= 10;
      Value |= op;
      // op: rj
      op = getMachineOpValue(MI, MI.getOperand(2), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: rd
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::BSTRPICK_W: {
      // op: msbw
      op = getMachineOpValue(MI, MI.getOperand(2), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 16;
      Value |= op;
      // op: lsbw
      op = getMachineOpValue(MI, MI.getOperand(3), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 10;
      Value |= op;
      // op: rj
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: rd
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::BSTRINS_W: {
      // op: msbw
      op = getMachineOpValue(MI, MI.getOperand(3), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 16;
      Value |= op;
      // op: lsbw
      op = getMachineOpValue(MI, MI.getOperand(4), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 10;
      Value |= op;
      // op: rj
      op = getMachineOpValue(MI, MI.getOperand(2), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: rd
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::BITREV_4B:
    case LoongArch::BITREV_8B:
    case LoongArch::BITREV_D:
    case LoongArch::BITREV_W:
    case LoongArch::CLO_D:
    case LoongArch::CLO_W:
    case LoongArch::CLZ_D:
    case LoongArch::CLZ_W:
    case LoongArch::CPUCFG:
    case LoongArch::CTO_D:
    case LoongArch::CTO_W:
    case LoongArch::CTZ_D:
    case LoongArch::CTZ_W:
    case LoongArch::EXT_W_B:
    case LoongArch::EXT_W_H:
    case LoongArch::IOCSRRD_B:
    case LoongArch::IOCSRRD_D:
    case LoongArch::IOCSRRD_H:
    case LoongArch::IOCSRRD_W:
    case LoongArch::IOCSRWR_B:
    case LoongArch::IOCSRWR_D:
    case LoongArch::IOCSRWR_H:
    case LoongArch::IOCSRWR_W:
    case LoongArch::RDTIMEH_W:
    case LoongArch::RDTIMEL_W:
    case LoongArch::RDTIME_D:
    case LoongArch::REVB_2H:
    case LoongArch::REVB_2W:
    case LoongArch::REVB_4H:
    case LoongArch::REVB_D:
    case LoongArch::REVH_2W:
    case LoongArch::REVH_D: {
      // op: rj
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: rd
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::INVTLB: {
      // op: rk
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 10;
      Value |= op;
      // op: rj
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: op
      op = getMachineOpValue(MI, MI.getOperand(2), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::ASRTGT_D:
    case LoongArch::ASRTLE_D: {
      // op: rk
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 10;
      Value |= op;
      // op: rj
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      break;
    }
    case LoongArch::AMADD_D:
    case LoongArch::AMADD_DB_D:
    case LoongArch::AMADD_DB_W:
    case LoongArch::AMADD_W:
    case LoongArch::AMAND_D:
    case LoongArch::AMAND_DB_D:
    case LoongArch::AMAND_DB_W:
    case LoongArch::AMAND_W:
    case LoongArch::AMMAX_D:
    case LoongArch::AMMAX_DB_D:
    case LoongArch::AMMAX_DB_DU:
    case LoongArch::AMMAX_DB_W:
    case LoongArch::AMMAX_DB_WU:
    case LoongArch::AMMAX_DU:
    case LoongArch::AMMAX_W:
    case LoongArch::AMMAX_WU:
    case LoongArch::AMMIN_D:
    case LoongArch::AMMIN_DB_D:
    case LoongArch::AMMIN_DB_DU:
    case LoongArch::AMMIN_DB_W:
    case LoongArch::AMMIN_DB_WU:
    case LoongArch::AMMIN_DU:
    case LoongArch::AMMIN_W:
    case LoongArch::AMMIN_WU:
    case LoongArch::AMOR_D:
    case LoongArch::AMOR_DB_D:
    case LoongArch::AMOR_DB_W:
    case LoongArch::AMOR_W:
    case LoongArch::AMSWAP_D:
    case LoongArch::AMSWAP_DB_D:
    case LoongArch::AMSWAP_DB_W:
    case LoongArch::AMSWAP_W:
    case LoongArch::AMXOR_D:
    case LoongArch::AMXOR_DB_D:
    case LoongArch::AMXOR_DB_W:
    case LoongArch::AMXOR_W: {
      // op: rk
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 10;
      Value |= op;
      // op: rj
      op = getMachineOpValue(MI, MI.getOperand(2), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: rd
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::FLDGT_D:
    case LoongArch::FLDGT_S:
    case LoongArch::FLDLE_D:
    case LoongArch::FLDLE_S:
    case LoongArch::FLDX_D:
    case LoongArch::FLDX_S:
    case LoongArch::FSTGT_D:
    case LoongArch::FSTGT_S:
    case LoongArch::FSTLE_D:
    case LoongArch::FSTLE_S:
    case LoongArch::FSTX_D:
    case LoongArch::FSTX_S: {
      // op: rk
      op = getMachineOpValue(MI, MI.getOperand(2), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 10;
      Value |= op;
      // op: rj
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: fd
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::PRELDX: {
      // op: rk
      op = getMachineOpValue(MI, MI.getOperand(2), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 10;
      Value |= op;
      // op: rj
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: imm5
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::ADD_D:
    case LoongArch::ADD_W:
    case LoongArch::AND:
    case LoongArch::ANDN:
    case LoongArch::CRCC_W_B_W:
    case LoongArch::CRCC_W_D_W:
    case LoongArch::CRCC_W_H_W:
    case LoongArch::CRCC_W_W_W:
    case LoongArch::CRC_W_B_W:
    case LoongArch::CRC_W_D_W:
    case LoongArch::CRC_W_H_W:
    case LoongArch::CRC_W_W_W:
    case LoongArch::DIV_D:
    case LoongArch::DIV_DU:
    case LoongArch::DIV_W:
    case LoongArch::DIV_WU:
    case LoongArch::LDGT_B:
    case LoongArch::LDGT_D:
    case LoongArch::LDGT_H:
    case LoongArch::LDGT_W:
    case LoongArch::LDLE_B:
    case LoongArch::LDLE_D:
    case LoongArch::LDLE_H:
    case LoongArch::LDLE_W:
    case LoongArch::LDX_B:
    case LoongArch::LDX_BU:
    case LoongArch::LDX_D:
    case LoongArch::LDX_H:
    case LoongArch::LDX_HU:
    case LoongArch::LDX_W:
    case LoongArch::LDX_WU:
    case LoongArch::MASKEQZ:
    case LoongArch::MASKNEZ:
    case LoongArch::MOD_D:
    case LoongArch::MOD_DU:
    case LoongArch::MOD_W:
    case LoongArch::MOD_WU:
    case LoongArch::MULH_D:
    case LoongArch::MULH_DU:
    case LoongArch::MULH_W:
    case LoongArch::MULH_WU:
    case LoongArch::MULW_D_W:
    case LoongArch::MULW_D_WU:
    case LoongArch::MUL_D:
    case LoongArch::MUL_W:
    case LoongArch::NOR:
    case LoongArch::OR:
    case LoongArch::ORN:
    case LoongArch::ROTR_D:
    case LoongArch::ROTR_W:
    case LoongArch::SLL_D:
    case LoongArch::SLL_W:
    case LoongArch::SLT:
    case LoongArch::SLTU:
    case LoongArch::SRA_D:
    case LoongArch::SRA_W:
    case LoongArch::SRL_D:
    case LoongArch::SRL_W:
    case LoongArch::STGT_B:
    case LoongArch::STGT_D:
    case LoongArch::STGT_H:
    case LoongArch::STGT_W:
    case LoongArch::STLE_B:
    case LoongArch::STLE_D:
    case LoongArch::STLE_H:
    case LoongArch::STLE_W:
    case LoongArch::STX_B:
    case LoongArch::STX_D:
    case LoongArch::STX_H:
    case LoongArch::STX_W:
    case LoongArch::SUB_D:
    case LoongArch::SUB_W:
    case LoongArch::XOR: {
      // op: rk
      op = getMachineOpValue(MI, MI.getOperand(2), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 10;
      Value |= op;
      // op: rj
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: rd
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::LDPTE: {
      // op: seq
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(255);
      op <<= 10;
      Value |= op;
      // op: rj
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      break;
    }
    case LoongArch::FMOV_D:
    case LoongArch::FMOV_S:
    case LoongArch::MOVCF2FR_S:
    case LoongArch::MOVCF2GR:
    case LoongArch::MOVFCSR2GR:
    case LoongArch::MOVFR2CF_S:
    case LoongArch::MOVFR2GR_D:
    case LoongArch::MOVFR2GR_S:
    case LoongArch::MOVFR2GR_S_64:
    case LoongArch::MOVFRH2GR_S:
    case LoongArch::MOVGR2CF:
    case LoongArch::MOVGR2FCSR:
    case LoongArch::MOVGR2FR_D:
    case LoongArch::MOVGR2FR_W:
    case LoongArch::MOVGR2FR_W_64: {
      // op: src
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: dst
      op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
    case LoongArch::MOVGR2FRH_W: {
      // op: src
      op = getMachineOpValue(MI, MI.getOperand(2), Fixups, STI);
      op &= UINT64_C(31);
      op <<= 5;
      Value |= op;
      // op: dst
      op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
      op &= UINT64_C(31);
      Value |= op;
      break;
    }
  default:
    std::string msg;
    raw_string_ostream Msg(msg);
    Msg << "Not supported instr: " << MI;
    report_fatal_error(Msg.str().c_str());
  }
  return Value;
}

