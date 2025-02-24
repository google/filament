#ifdef GET_BANKEDREG_DECL
enum BankedRegValues {
  elr_hyp = 0,
  lr_abt = 1,
  lr_fiq = 2,
  lr_irq = 3,
  lr_mon = 4,
  lr_svc = 5,
  lr_und = 6,
  lr_usr = 7,
  r10_fiq = 8,
  r10_usr = 9,
  r11_fiq = 10,
  r11_usr = 11,
  r12_fiq = 12,
  r12_usr = 13,
  r8_fiq = 14,
  r8_usr = 15,
  r9_fiq = 16,
  r9_usr = 17,
  sp_abt = 18,
  sp_fiq = 19,
  sp_hyp = 20,
  sp_irq = 21,
  sp_mon = 22,
  sp_svc = 23,
  sp_und = 24,
  sp_usr = 25,
  spsr_abt = 26,
  spsr_fiq = 27,
  spsr_hyp = 28,
  spsr_irq = 29,
  spsr_mon = 30,
  spsr_svc = 31,
  spsr_und = 32,
};
#endif

#ifdef GET_MCLASSSYSREG_DECL
enum MClassSysRegValues {
  apsr_g = 1024,
  apsr_nzcvqg = 3072,
  iapsr_g = 1025,
  iapsr_nzcvqg = 3073,
  eapsr_g = 1026,
  eapsr_nzcvqg = 3074,
  xpsr_g = 1027,
  xpsr_nzcvqg = 3075,
  apsr = 2048,
  apsr_nzcvq = 2048,
  iapsr = 2049,
  iapsr_nzcvq = 2049,
  eapsr = 2050,
  eapsr_nzcvq = 2050,
  xpsr = 2051,
  xpsr_nzcvq = 2051,
  ipsr = 2053,
  epsr = 2054,
  iepsr = 2055,
  msp = 2056,
  psp = 2057,
  msplim = 2058,
  psplim = 2059,
  primask = 2064,
  basepri = 2065,
  basepri_max = 2066,
  faultmask = 2067,
  control = 2068,
  msp_ns = 2184,
  psp_ns = 2185,
  msplim_ns = 2186,
  psplim_ns = 2187,
  primask_ns = 2192,
  basepri_ns = 2193,
  faultmask_ns = 2195,
  control_ns = 2196,
  sp_ns = 2200,
  pac_key_p_0 = 2080,
  pac_key_p_1 = 2081,
  pac_key_p_2 = 2082,
  pac_key_p_3 = 2083,
  pac_key_u_0 = 2084,
  pac_key_u_1 = 2085,
  pac_key_u_2 = 2086,
  pac_key_u_3 = 2087,
  pac_key_p_0_ns = 2208,
  pac_key_p_1_ns = 2209,
  pac_key_p_2_ns = 2210,
  pac_key_p_3_ns = 2211,
  pac_key_u_0_ns = 2212,
  pac_key_u_1_ns = 2213,
  pac_key_u_2_ns = 2214,
  pac_key_u_3_ns = 2215,
};
#endif

#ifdef GET_BANKEDREG_DECL
const BankedReg *lookupBankedRegByName(StringRef Name);
const BankedReg *lookupBankedRegByEncoding(uint8_t Encoding);
#endif

#ifdef GET_BANKEDREG_IMPL
constexpr BankedReg BankedRegsList[] = {
  { "elr_hyp", 0x1E }, // 0
  { "lr_abt", 0x14 }, // 1
  { "lr_fiq", 0xE }, // 2
  { "lr_irq", 0x10 }, // 3
  { "lr_mon", 0x1C }, // 4
  { "lr_svc", 0x12 }, // 5
  { "lr_und", 0x16 }, // 6
  { "lr_usr", 0x6 }, // 7
  { "r10_fiq", 0xA }, // 8
  { "r10_usr", 0x2 }, // 9
  { "r11_fiq", 0xB }, // 10
  { "r11_usr", 0x3 }, // 11
  { "r12_fiq", 0xC }, // 12
  { "r12_usr", 0x4 }, // 13
  { "r8_fiq", 0x8 }, // 14
  { "r8_usr", 0x0 }, // 15
  { "r9_fiq", 0x9 }, // 16
  { "r9_usr", 0x1 }, // 17
  { "spsr_abt", 0x34 }, // 18
  { "spsr_fiq", 0x2E }, // 19
  { "spsr_hyp", 0x3E }, // 20
  { "spsr_irq", 0x30 }, // 21
  { "spsr_mon", 0x3C }, // 22
  { "spsr_svc", 0x32 }, // 23
  { "spsr_und", 0x36 }, // 24
  { "sp_abt", 0x15 }, // 25
  { "sp_fiq", 0xD }, // 26
  { "sp_hyp", 0x1F }, // 27
  { "sp_irq", 0x11 }, // 28
  { "sp_mon", 0x1D }, // 29
  { "sp_svc", 0x13 }, // 30
  { "sp_und", 0x17 }, // 31
  { "sp_usr", 0x5 }, // 32
 };

const BankedReg *lookupBankedRegByName(StringRef Name) {
  struct IndexType {
    const char * Name;
    unsigned _index;
  };
  static const struct IndexType Index[] = {
    { "ELR_HYP", 0 },
    { "LR_ABT", 1 },
    { "LR_FIQ", 2 },
    { "LR_IRQ", 3 },
    { "LR_MON", 4 },
    { "LR_SVC", 5 },
    { "LR_UND", 6 },
    { "LR_USR", 7 },
    { "R10_FIQ", 8 },
    { "R10_USR", 9 },
    { "R11_FIQ", 10 },
    { "R11_USR", 11 },
    { "R12_FIQ", 12 },
    { "R12_USR", 13 },
    { "R8_FIQ", 14 },
    { "R8_USR", 15 },
    { "R9_FIQ", 16 },
    { "R9_USR", 17 },
    { "SPSR_ABT", 18 },
    { "SPSR_FIQ", 19 },
    { "SPSR_HYP", 20 },
    { "SPSR_IRQ", 21 },
    { "SPSR_MON", 22 },
    { "SPSR_SVC", 23 },
    { "SPSR_UND", 24 },
    { "SP_ABT", 25 },
    { "SP_FIQ", 26 },
    { "SP_HYP", 27 },
    { "SP_IRQ", 28 },
    { "SP_MON", 29 },
    { "SP_SVC", 30 },
    { "SP_UND", 31 },
    { "SP_USR", 32 },
  };

  struct KeyType {
    std::string Name;
  };
  KeyType Key = {Name.upper()};
  auto Table = ArrayRef(Index);
  auto Idx = std::lower_bound(Table.begin(), Table.end(), Key,
    [](const IndexType &LHS, const KeyType &RHS) {
      int CmpName = StringRef(LHS.Name).compare(RHS.Name);
      if (CmpName < 0) return true;
      if (CmpName > 0) return false;
      return false;
    });

  if (Idx == Table.end() ||
      Key.Name != Idx->Name)
    return nullptr;
  return &BankedRegsList[Idx->_index];
}

const BankedReg *lookupBankedRegByEncoding(uint8_t Encoding) {
  struct IndexType {
    uint8_t Encoding;
    unsigned _index;
  };
  static const struct IndexType Index[] = {
    { 0x0, 15 },
    { 0x1, 17 },
    { 0x2, 9 },
    { 0x3, 11 },
    { 0x4, 13 },
    { 0x5, 32 },
    { 0x6, 7 },
    { 0x8, 14 },
    { 0x9, 16 },
    { 0xA, 8 },
    { 0xB, 10 },
    { 0xC, 12 },
    { 0xD, 26 },
    { 0xE, 2 },
    { 0x10, 3 },
    { 0x11, 28 },
    { 0x12, 5 },
    { 0x13, 30 },
    { 0x14, 1 },
    { 0x15, 25 },
    { 0x16, 6 },
    { 0x17, 31 },
    { 0x1C, 4 },
    { 0x1D, 29 },
    { 0x1E, 0 },
    { 0x1F, 27 },
    { 0x2E, 19 },
    { 0x30, 21 },
    { 0x32, 23 },
    { 0x34, 18 },
    { 0x36, 24 },
    { 0x3C, 22 },
    { 0x3E, 20 },
  };

  struct KeyType {
    uint8_t Encoding;
  };
  KeyType Key = {Encoding};
  auto Table = ArrayRef(Index);
  auto Idx = std::lower_bound(Table.begin(), Table.end(), Key,
    [](const IndexType &LHS, const KeyType &RHS) {
      if (LHS.Encoding < RHS.Encoding)
        return true;
      if (LHS.Encoding > RHS.Encoding)
        return false;
      return false;
    });

  if (Idx == Table.end() ||
      Key.Encoding != Idx->Encoding)
    return nullptr;
  return &BankedRegsList[Idx->_index];
}
#endif

#ifdef GET_MCLASSSYSREG_DECL
const MClassSysReg *lookupMClassSysRegByName(StringRef Name);
const MClassSysReg *lookupMClassSysRegByM1Encoding12(uint16_t M1Encoding12);
const MClassSysReg *lookupMClassSysRegByM2M3Encoding8(uint16_t M2M3Encoding8);
const MClassSysReg *lookupMClassSysRegByEncoding(uint16_t Encoding);
#endif

#ifdef GET_MCLASSSYSREG_IMPL
constexpr MClassSysReg MClassSysRegsList[] = {
  { "apsr", 0x800, 0x100, 0x800,  {}  }, // 0
  { "apsr_g", 0x400, 0x0, 0x400,  {ARM::FeatureDSP}  }, // 1
  { "apsr_nzcvq", 0x1800, 0x200, 0x800,  {}  }, // 2
  { "apsr_nzcvqg", 0xC00, 0x300, 0xC00,  {ARM::FeatureDSP}  }, // 3
  { "basepri", 0x811, 0x111, 0x811,  {ARM::HasV7Ops}  }, // 4
  { "basepri_max", 0x812, 0x112, 0x812,  {ARM::HasV7Ops}  }, // 5
  { "basepri_ns", 0x891, 0x191, 0x891,  {ARM::Feature8MSecExt, ARM::HasV7Ops}  }, // 6
  { "control", 0x814, 0x114, 0x814,  {}  }, // 7
  { "control_ns", 0x894, 0x194, 0x894,  {ARM::Feature8MSecExt}  }, // 8
  { "eapsr", 0x802, 0x102, 0x802,  {}  }, // 9
  { "eapsr_g", 0x402, 0x2, 0x402,  {ARM::FeatureDSP}  }, // 10
  { "eapsr_nzcvq", 0x1802, 0x202, 0x802,  {}  }, // 11
  { "eapsr_nzcvqg", 0xC02, 0x302, 0xC02,  {ARM::FeatureDSP}  }, // 12
  { "epsr", 0x806, 0x106, 0x806,  {}  }, // 13
  { "faultmask", 0x813, 0x113, 0x813,  {ARM::HasV7Ops}  }, // 14
  { "faultmask_ns", 0x893, 0x193, 0x893,  {ARM::Feature8MSecExt, ARM::HasV7Ops}  }, // 15
  { "iapsr", 0x801, 0x101, 0x801,  {}  }, // 16
  { "iapsr_g", 0x401, 0x1, 0x401,  {ARM::FeatureDSP}  }, // 17
  { "iapsr_nzcvq", 0x1801, 0x201, 0x801,  {}  }, // 18
  { "iapsr_nzcvqg", 0xC01, 0x301, 0xC01,  {ARM::FeatureDSP}  }, // 19
  { "iepsr", 0x807, 0x107, 0x807,  {}  }, // 20
  { "ipsr", 0x805, 0x105, 0x805,  {}  }, // 21
  { "msp", 0x808, 0x108, 0x808,  {}  }, // 22
  { "msplim", 0x80A, 0x10A, 0x80A,  {ARM::HasV8MBaselineOps}  }, // 23
  { "msplim_ns", 0x88A, 0x18A, 0x88A,  {ARM::Feature8MSecExt, ARM::HasV8MBaselineOps}  }, // 24
  { "msp_ns", 0x888, 0x188, 0x888,  {ARM::Feature8MSecExt}  }, // 25
  { "pac_key_p_0", 0x820, 0x120, 0x820,  {ARM::FeaturePACBTI}  }, // 26
  { "pac_key_p_0_ns", 0x8A0, 0x1A0, 0x8A0,  {ARM::FeaturePACBTI}  }, // 27
  { "pac_key_p_1", 0x821, 0x121, 0x821,  {ARM::FeaturePACBTI}  }, // 28
  { "pac_key_p_1_ns", 0x8A1, 0x1A1, 0x8A1,  {ARM::FeaturePACBTI}  }, // 29
  { "pac_key_p_2", 0x822, 0x122, 0x822,  {ARM::FeaturePACBTI}  }, // 30
  { "pac_key_p_2_ns", 0x8A2, 0x1A2, 0x8A2,  {ARM::FeaturePACBTI}  }, // 31
  { "pac_key_p_3", 0x823, 0x123, 0x823,  {ARM::FeaturePACBTI}  }, // 32
  { "pac_key_p_3_ns", 0x8A3, 0x1A3, 0x8A3,  {ARM::FeaturePACBTI}  }, // 33
  { "pac_key_u_0", 0x824, 0x124, 0x824,  {ARM::FeaturePACBTI}  }, // 34
  { "pac_key_u_0_ns", 0x8A4, 0x1A4, 0x8A4,  {ARM::FeaturePACBTI}  }, // 35
  { "pac_key_u_1", 0x825, 0x125, 0x825,  {ARM::FeaturePACBTI}  }, // 36
  { "pac_key_u_1_ns", 0x8A5, 0x1A5, 0x8A5,  {ARM::FeaturePACBTI}  }, // 37
  { "pac_key_u_2", 0x826, 0x126, 0x826,  {ARM::FeaturePACBTI}  }, // 38
  { "pac_key_u_2_ns", 0x8A6, 0x1A6, 0x8A6,  {ARM::FeaturePACBTI}  }, // 39
  { "pac_key_u_3", 0x827, 0x127, 0x827,  {ARM::FeaturePACBTI}  }, // 40
  { "pac_key_u_3_ns", 0x8A7, 0x1A7, 0x8A7,  {ARM::FeaturePACBTI}  }, // 41
  { "primask", 0x810, 0x110, 0x810,  {}  }, // 42
  { "primask_ns", 0x890, 0x190, 0x890,  {}  }, // 43
  { "psp", 0x809, 0x109, 0x809,  {}  }, // 44
  { "psplim", 0x80B, 0x10B, 0x80B,  {ARM::HasV8MBaselineOps}  }, // 45
  { "psplim_ns", 0x88B, 0x18B, 0x88B,  {ARM::Feature8MSecExt, ARM::HasV8MBaselineOps}  }, // 46
  { "psp_ns", 0x889, 0x189, 0x889,  {ARM::Feature8MSecExt}  }, // 47
  { "sp_ns", 0x898, 0x198, 0x898,  {ARM::Feature8MSecExt}  }, // 48
  { "xpsr", 0x803, 0x103, 0x803,  {}  }, // 49
  { "xpsr_g", 0x403, 0x3, 0x403,  {ARM::FeatureDSP}  }, // 50
  { "xpsr_nzcvq", 0x1803, 0x203, 0x803,  {}  }, // 51
  { "xpsr_nzcvqg", 0xC03, 0x303, 0xC03,  {ARM::FeatureDSP}  }, // 52
 };

const MClassSysReg *lookupMClassSysRegByName(StringRef Name) {
  struct IndexType {
    const char * Name;
    unsigned _index;
  };
  static const struct IndexType Index[] = {
    { "APSR", 0 },
    { "APSR_G", 1 },
    { "APSR_NZCVQ", 2 },
    { "APSR_NZCVQG", 3 },
    { "BASEPRI", 4 },
    { "BASEPRI_MAX", 5 },
    { "BASEPRI_NS", 6 },
    { "CONTROL", 7 },
    { "CONTROL_NS", 8 },
    { "EAPSR", 9 },
    { "EAPSR_G", 10 },
    { "EAPSR_NZCVQ", 11 },
    { "EAPSR_NZCVQG", 12 },
    { "EPSR", 13 },
    { "FAULTMASK", 14 },
    { "FAULTMASK_NS", 15 },
    { "IAPSR", 16 },
    { "IAPSR_G", 17 },
    { "IAPSR_NZCVQ", 18 },
    { "IAPSR_NZCVQG", 19 },
    { "IEPSR", 20 },
    { "IPSR", 21 },
    { "MSP", 22 },
    { "MSPLIM", 23 },
    { "MSPLIM_NS", 24 },
    { "MSP_NS", 25 },
    { "PAC_KEY_P_0", 26 },
    { "PAC_KEY_P_0_NS", 27 },
    { "PAC_KEY_P_1", 28 },
    { "PAC_KEY_P_1_NS", 29 },
    { "PAC_KEY_P_2", 30 },
    { "PAC_KEY_P_2_NS", 31 },
    { "PAC_KEY_P_3", 32 },
    { "PAC_KEY_P_3_NS", 33 },
    { "PAC_KEY_U_0", 34 },
    { "PAC_KEY_U_0_NS", 35 },
    { "PAC_KEY_U_1", 36 },
    { "PAC_KEY_U_1_NS", 37 },
    { "PAC_KEY_U_2", 38 },
    { "PAC_KEY_U_2_NS", 39 },
    { "PAC_KEY_U_3", 40 },
    { "PAC_KEY_U_3_NS", 41 },
    { "PRIMASK", 42 },
    { "PRIMASK_NS", 43 },
    { "PSP", 44 },
    { "PSPLIM", 45 },
    { "PSPLIM_NS", 46 },
    { "PSP_NS", 47 },
    { "SP_NS", 48 },
    { "XPSR", 49 },
    { "XPSR_G", 50 },
    { "XPSR_NZCVQ", 51 },
    { "XPSR_NZCVQG", 52 },
  };

  struct KeyType {
    std::string Name;
  };
  KeyType Key = {Name.upper()};
  auto Table = ArrayRef(Index);
  auto Idx = std::lower_bound(Table.begin(), Table.end(), Key,
    [](const IndexType &LHS, const KeyType &RHS) {
      int CmpName = StringRef(LHS.Name).compare(RHS.Name);
      if (CmpName < 0) return true;
      if (CmpName > 0) return false;
      return false;
    });

  if (Idx == Table.end() ||
      Key.Name != Idx->Name)
    return nullptr;
  return &MClassSysRegsList[Idx->_index];
}

const MClassSysReg *lookupMClassSysRegByM1Encoding12(uint16_t M1Encoding12) {
  struct IndexType {
    uint16_t M1Encoding12;
    unsigned _index;
  };
  static const struct IndexType Index[] = {
    { 0x400, 1 },
    { 0x401, 17 },
    { 0x402, 10 },
    { 0x403, 50 },
    { 0x800, 0 },
    { 0x801, 16 },
    { 0x802, 9 },
    { 0x803, 49 },
    { 0x805, 21 },
    { 0x806, 13 },
    { 0x807, 20 },
    { 0x808, 22 },
    { 0x809, 44 },
    { 0x80A, 23 },
    { 0x80B, 45 },
    { 0x810, 42 },
    { 0x811, 4 },
    { 0x812, 5 },
    { 0x813, 14 },
    { 0x814, 7 },
    { 0x820, 26 },
    { 0x821, 28 },
    { 0x822, 30 },
    { 0x823, 32 },
    { 0x824, 34 },
    { 0x825, 36 },
    { 0x826, 38 },
    { 0x827, 40 },
    { 0x888, 25 },
    { 0x889, 47 },
    { 0x88A, 24 },
    { 0x88B, 46 },
    { 0x890, 43 },
    { 0x891, 6 },
    { 0x893, 15 },
    { 0x894, 8 },
    { 0x898, 48 },
    { 0x8A0, 27 },
    { 0x8A1, 29 },
    { 0x8A2, 31 },
    { 0x8A3, 33 },
    { 0x8A4, 35 },
    { 0x8A5, 37 },
    { 0x8A6, 39 },
    { 0x8A7, 41 },
    { 0xC00, 3 },
    { 0xC01, 19 },
    { 0xC02, 12 },
    { 0xC03, 52 },
    { 0x1800, 2 },
    { 0x1801, 18 },
    { 0x1802, 11 },
    { 0x1803, 51 },
  };

  struct KeyType {
    uint16_t M1Encoding12;
  };
  KeyType Key = {M1Encoding12};
  auto Table = ArrayRef(Index);
  auto Idx = std::lower_bound(Table.begin(), Table.end(), Key,
    [](const IndexType &LHS, const KeyType &RHS) {
      if (LHS.M1Encoding12 < RHS.M1Encoding12)
        return true;
      if (LHS.M1Encoding12 > RHS.M1Encoding12)
        return false;
      return false;
    });

  if (Idx == Table.end() ||
      Key.M1Encoding12 != Idx->M1Encoding12)
    return nullptr;
  return &MClassSysRegsList[Idx->_index];
}

const MClassSysReg *lookupMClassSysRegByM2M3Encoding8(uint16_t M2M3Encoding8) {
  struct IndexType {
    uint16_t M2M3Encoding8;
    unsigned _index;
  };
  static const struct IndexType Index[] = {
    { 0x0, 1 },
    { 0x1, 17 },
    { 0x2, 10 },
    { 0x3, 50 },
    { 0x100, 0 },
    { 0x101, 16 },
    { 0x102, 9 },
    { 0x103, 49 },
    { 0x105, 21 },
    { 0x106, 13 },
    { 0x107, 20 },
    { 0x108, 22 },
    { 0x109, 44 },
    { 0x10A, 23 },
    { 0x10B, 45 },
    { 0x110, 42 },
    { 0x111, 4 },
    { 0x112, 5 },
    { 0x113, 14 },
    { 0x114, 7 },
    { 0x120, 26 },
    { 0x121, 28 },
    { 0x122, 30 },
    { 0x123, 32 },
    { 0x124, 34 },
    { 0x125, 36 },
    { 0x126, 38 },
    { 0x127, 40 },
    { 0x188, 25 },
    { 0x189, 47 },
    { 0x18A, 24 },
    { 0x18B, 46 },
    { 0x190, 43 },
    { 0x191, 6 },
    { 0x193, 15 },
    { 0x194, 8 },
    { 0x198, 48 },
    { 0x1A0, 27 },
    { 0x1A1, 29 },
    { 0x1A2, 31 },
    { 0x1A3, 33 },
    { 0x1A4, 35 },
    { 0x1A5, 37 },
    { 0x1A6, 39 },
    { 0x1A7, 41 },
    { 0x200, 2 },
    { 0x201, 18 },
    { 0x202, 11 },
    { 0x203, 51 },
    { 0x300, 3 },
    { 0x301, 19 },
    { 0x302, 12 },
    { 0x303, 52 },
  };

  struct KeyType {
    uint16_t M2M3Encoding8;
  };
  KeyType Key = {M2M3Encoding8};
  auto Table = ArrayRef(Index);
  auto Idx = std::lower_bound(Table.begin(), Table.end(), Key,
    [](const IndexType &LHS, const KeyType &RHS) {
      if (LHS.M2M3Encoding8 < RHS.M2M3Encoding8)
        return true;
      if (LHS.M2M3Encoding8 > RHS.M2M3Encoding8)
        return false;
      return false;
    });

  if (Idx == Table.end() ||
      Key.M2M3Encoding8 != Idx->M2M3Encoding8)
    return nullptr;
  return &MClassSysRegsList[Idx->_index];
}

const MClassSysReg *lookupMClassSysRegByEncoding(uint16_t Encoding) {
  struct IndexType {
    uint16_t Encoding;
    unsigned _index;
  };
  static const struct IndexType Index[] = {
    { 0x400, 1 },
    { 0x401, 17 },
    { 0x402, 10 },
    { 0x403, 50 },
    { 0x800, 0 },
    { 0x800, 2 },
    { 0x801, 16 },
    { 0x801, 18 },
    { 0x802, 9 },
    { 0x802, 11 },
    { 0x803, 49 },
    { 0x803, 51 },
    { 0x805, 21 },
    { 0x806, 13 },
    { 0x807, 20 },
    { 0x808, 22 },
    { 0x809, 44 },
    { 0x80A, 23 },
    { 0x80B, 45 },
    { 0x810, 42 },
    { 0x811, 4 },
    { 0x812, 5 },
    { 0x813, 14 },
    { 0x814, 7 },
    { 0x820, 26 },
    { 0x821, 28 },
    { 0x822, 30 },
    { 0x823, 32 },
    { 0x824, 34 },
    { 0x825, 36 },
    { 0x826, 38 },
    { 0x827, 40 },
    { 0x888, 25 },
    { 0x889, 47 },
    { 0x88A, 24 },
    { 0x88B, 46 },
    { 0x890, 43 },
    { 0x891, 6 },
    { 0x893, 15 },
    { 0x894, 8 },
    { 0x898, 48 },
    { 0x8A0, 27 },
    { 0x8A1, 29 },
    { 0x8A2, 31 },
    { 0x8A3, 33 },
    { 0x8A4, 35 },
    { 0x8A5, 37 },
    { 0x8A6, 39 },
    { 0x8A7, 41 },
    { 0xC00, 3 },
    { 0xC01, 19 },
    { 0xC02, 12 },
    { 0xC03, 52 },
  };

  struct KeyType {
    uint16_t Encoding;
  };
  KeyType Key = {Encoding};
  auto Table = ArrayRef(Index);
  auto Idx = std::lower_bound(Table.begin(), Table.end(), Key,
    [](const IndexType &LHS, const KeyType &RHS) {
      if (LHS.Encoding < RHS.Encoding)
        return true;
      if (LHS.Encoding > RHS.Encoding)
        return false;
      return false;
    });

  if (Idx == Table.end() ||
      Key.Encoding != Idx->Encoding)
    return nullptr;
  return &MClassSysRegsList[Idx->_index];
}
#endif

#undef GET_BANKEDREG_DECL
#undef GET_BANKEDREG_IMPL
#undef GET_MCLASSSYSREG_DECL
#undef GET_MCLASSSYSREG_IMPL
