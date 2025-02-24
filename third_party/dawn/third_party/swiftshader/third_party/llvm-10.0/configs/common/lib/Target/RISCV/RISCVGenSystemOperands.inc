#ifdef GET_SysRegsList_DECL
const SysReg *lookupSysRegByEncoding(uint16_t Encoding);
const SysReg *lookupSysRegByName(StringRef Name);
#endif

#ifdef GET_SysRegsList_IMPL
constexpr SysReg SysRegsList[] = {
  { "ustatus", 0x0,  {} , false }, // 0
  { "fflags", 0x1,  {} , false }, // 1
  { "frm", 0x2,  {} , false }, // 2
  { "fcsr", 0x3,  {} , false }, // 3
  { "uie", 0x4,  {} , false }, // 4
  { "utvec", 0x5,  {} , false }, // 5
  { "uscratch", 0x40,  {} , false }, // 6
  { "uepc", 0x41,  {} , false }, // 7
  { "ucause", 0x42,  {} , false }, // 8
  { "utval", 0x43,  {} , false }, // 9
  { "uip", 0x44,  {} , false }, // 10
  { "sstatus", 0x100,  {} , false }, // 11
  { "sedeleg", 0x102,  {} , false }, // 12
  { "sideleg", 0x103,  {} , false }, // 13
  { "sie", 0x104,  {} , false }, // 14
  { "stvec", 0x105,  {} , false }, // 15
  { "scounteren", 0x106,  {} , false }, // 16
  { "sscratch", 0x140,  {} , false }, // 17
  { "sepc", 0x141,  {} , false }, // 18
  { "scause", 0x142,  {} , false }, // 19
  { "stval", 0x143,  {} , false }, // 20
  { "sip", 0x144,  {} , false }, // 21
  { "satp", 0x180,  {} , false }, // 22
  { "mstatus", 0x300,  {} , false }, // 23
  { "misa", 0x301,  {} , false }, // 24
  { "medeleg", 0x302,  {} , false }, // 25
  { "mideleg", 0x303,  {} , false }, // 26
  { "mie", 0x304,  {} , false }, // 27
  { "mtvec", 0x305,  {} , false }, // 28
  { "mcounteren", 0x306,  {} , false }, // 29
  { "mhpmevent3", 0x323,  {} , false }, // 30
  { "mhpmevent4", 0x324,  {} , false }, // 31
  { "mhpmevent5", 0x325,  {} , false }, // 32
  { "mhpmevent6", 0x326,  {} , false }, // 33
  { "mhpmevent7", 0x327,  {} , false }, // 34
  { "mhpmevent8", 0x328,  {} , false }, // 35
  { "mhpmevent9", 0x329,  {} , false }, // 36
  { "mhpmevent10", 0x32A,  {} , false }, // 37
  { "mhpmevent11", 0x32B,  {} , false }, // 38
  { "mhpmevent12", 0x32C,  {} , false }, // 39
  { "mhpmevent13", 0x32D,  {} , false }, // 40
  { "mhpmevent14", 0x32E,  {} , false }, // 41
  { "mhpmevent15", 0x32F,  {} , false }, // 42
  { "mhpmevent16", 0x330,  {} , false }, // 43
  { "mhpmevent17", 0x331,  {} , false }, // 44
  { "mhpmevent18", 0x332,  {} , false }, // 45
  { "mhpmevent19", 0x333,  {} , false }, // 46
  { "mhpmevent20", 0x334,  {} , false }, // 47
  { "mhpmevent21", 0x335,  {} , false }, // 48
  { "mhpmevent22", 0x336,  {} , false }, // 49
  { "mhpmevent23", 0x337,  {} , false }, // 50
  { "mhpmevent24", 0x338,  {} , false }, // 51
  { "mhpmevent25", 0x339,  {} , false }, // 52
  { "mhpmevent26", 0x33A,  {} , false }, // 53
  { "mhpmevent27", 0x33B,  {} , false }, // 54
  { "mhpmevent28", 0x33C,  {} , false }, // 55
  { "mhpmevent29", 0x33D,  {} , false }, // 56
  { "mhpmevent30", 0x33E,  {} , false }, // 57
  { "mhpmevent31", 0x33F,  {} , false }, // 58
  { "mscratch", 0x340,  {} , false }, // 59
  { "mepc", 0x341,  {} , false }, // 60
  { "mcause", 0x342,  {} , false }, // 61
  { "mtval", 0x343,  {} , false }, // 62
  { "mip", 0x344,  {} , false }, // 63
  { "pmpcfg0", 0x3A0,  {} , false }, // 64
  { "pmpcfg1", 0x3A1,  {} , true }, // 65
  { "pmpcfg2", 0x3A2,  {} , false }, // 66
  { "pmpcfg3", 0x3A3,  {} , true }, // 67
  { "pmpaddr0", 0x3B0,  {} , false }, // 68
  { "pmpaddr1", 0x3B1,  {} , false }, // 69
  { "pmpaddr2", 0x3B2,  {} , false }, // 70
  { "pmpaddr3", 0x3B3,  {} , false }, // 71
  { "pmpaddr4", 0x3B4,  {} , false }, // 72
  { "pmpaddr5", 0x3B5,  {} , false }, // 73
  { "pmpaddr6", 0x3B6,  {} , false }, // 74
  { "pmpaddr7", 0x3B7,  {} , false }, // 75
  { "pmpaddr8", 0x3B8,  {} , false }, // 76
  { "pmpaddr9", 0x3B9,  {} , false }, // 77
  { "pmpaddr10", 0x3BA,  {} , false }, // 78
  { "pmpaddr11", 0x3BB,  {} , false }, // 79
  { "pmpaddr12", 0x3BC,  {} , false }, // 80
  { "pmpaddr13", 0x3BD,  {} , false }, // 81
  { "pmpaddr14", 0x3BE,  {} , false }, // 82
  { "pmpaddr15", 0x3BF,  {} , false }, // 83
  { "tselect", 0x7A0,  {} , false }, // 84
  { "tdata1", 0x7A1,  {} , false }, // 85
  { "tdata2", 0x7A2,  {} , false }, // 86
  { "tdata3", 0x7A3,  {} , false }, // 87
  { "dcsr", 0x7B0,  {} , false }, // 88
  { "dpc", 0x7B1,  {} , false }, // 89
  { "dscratch", 0x7B2,  {} , false }, // 90
  { "mcycle", 0xB00,  {} , false }, // 91
  { "minstret", 0xB02,  {} , false }, // 92
  { "mhpmcounter3", 0xB03,  {} , false }, // 93
  { "mhpmcounter4", 0xB04,  {} , false }, // 94
  { "mhpmcounter5", 0xB05,  {} , false }, // 95
  { "mhpmcounter6", 0xB06,  {} , false }, // 96
  { "mhpmcounter7", 0xB07,  {} , false }, // 97
  { "mhpmcounter8", 0xB08,  {} , false }, // 98
  { "mhpmcounter9", 0xB09,  {} , false }, // 99
  { "mhpmcounter10", 0xB0A,  {} , false }, // 100
  { "mhpmcounter11", 0xB0B,  {} , false }, // 101
  { "mhpmcounter12", 0xB0C,  {} , false }, // 102
  { "mhpmcounter13", 0xB0D,  {} , false }, // 103
  { "mhpmcounter14", 0xB0E,  {} , false }, // 104
  { "mhpmcounter15", 0xB0F,  {} , false }, // 105
  { "mhpmcounter16", 0xB10,  {} , false }, // 106
  { "mhpmcounter17", 0xB11,  {} , false }, // 107
  { "mhpmcounter18", 0xB12,  {} , false }, // 108
  { "mhpmcounter19", 0xB13,  {} , false }, // 109
  { "mhpmcounter20", 0xB14,  {} , false }, // 110
  { "mhpmcounter21", 0xB15,  {} , false }, // 111
  { "mhpmcounter22", 0xB16,  {} , false }, // 112
  { "mhpmcounter23", 0xB17,  {} , false }, // 113
  { "mhpmcounter24", 0xB18,  {} , false }, // 114
  { "mhpmcounter25", 0xB19,  {} , false }, // 115
  { "mhpmcounter26", 0xB1A,  {} , false }, // 116
  { "mhpmcounter27", 0xB1B,  {} , false }, // 117
  { "mhpmcounter28", 0xB1C,  {} , false }, // 118
  { "mhpmcounter29", 0xB1D,  {} , false }, // 119
  { "mhpmcounter30", 0xB1E,  {} , false }, // 120
  { "mhpmcounter31", 0xB1F,  {} , false }, // 121
  { "mcycleh", 0xB80,  {} , true }, // 122
  { "minstreth", 0xB82,  {} , true }, // 123
  { "mhpmcounter3h", 0xB83,  {} , true }, // 124
  { "mhpmcounter4h", 0xB84,  {} , true }, // 125
  { "mhpmcounter5h", 0xB85,  {} , true }, // 126
  { "mhpmcounter6h", 0xB86,  {} , true }, // 127
  { "mhpmcounter7h", 0xB87,  {} , true }, // 128
  { "mhpmcounter8h", 0xB88,  {} , true }, // 129
  { "mhpmcounter9h", 0xB89,  {} , true }, // 130
  { "mhpmcounter10h", 0xB8A,  {} , true }, // 131
  { "mhpmcounter11h", 0xB8B,  {} , true }, // 132
  { "mhpmcounter12h", 0xB8C,  {} , true }, // 133
  { "mhpmcounter13h", 0xB8D,  {} , true }, // 134
  { "mhpmcounter14h", 0xB8E,  {} , true }, // 135
  { "mhpmcounter15h", 0xB8F,  {} , true }, // 136
  { "mhpmcounter16h", 0xB90,  {} , true }, // 137
  { "mhpmcounter17h", 0xB91,  {} , true }, // 138
  { "mhpmcounter18h", 0xB92,  {} , true }, // 139
  { "mhpmcounter19h", 0xB93,  {} , true }, // 140
  { "mhpmcounter20h", 0xB94,  {} , true }, // 141
  { "mhpmcounter21h", 0xB95,  {} , true }, // 142
  { "mhpmcounter22h", 0xB96,  {} , true }, // 143
  { "mhpmcounter23h", 0xB97,  {} , true }, // 144
  { "mhpmcounter24h", 0xB98,  {} , true }, // 145
  { "mhpmcounter25h", 0xB99,  {} , true }, // 146
  { "mhpmcounter26h", 0xB9A,  {} , true }, // 147
  { "mhpmcounter27h", 0xB9B,  {} , true }, // 148
  { "mhpmcounter28h", 0xB9C,  {} , true }, // 149
  { "mhpmcounter29h", 0xB9D,  {} , true }, // 150
  { "mhpmcounter30h", 0xB9E,  {} , true }, // 151
  { "mhpmcounter31h", 0xB9F,  {} , true }, // 152
  { "cycle", 0xC00,  {} , false }, // 153
  { "time", 0xC01,  {} , false }, // 154
  { "instret", 0xC02,  {} , false }, // 155
  { "hpmcounter3", 0xC03,  {} , false }, // 156
  { "hpmcounter4", 0xC04,  {} , false }, // 157
  { "hpmcounter5", 0xC05,  {} , false }, // 158
  { "hpmcounter6", 0xC06,  {} , false }, // 159
  { "hpmcounter7", 0xC07,  {} , false }, // 160
  { "hpmcounter8", 0xC08,  {} , false }, // 161
  { "hpmcounter9", 0xC09,  {} , false }, // 162
  { "hpmcounter10", 0xC0A,  {} , false }, // 163
  { "hpmcounter11", 0xC0B,  {} , false }, // 164
  { "hpmcounter12", 0xC0C,  {} , false }, // 165
  { "hpmcounter13", 0xC0D,  {} , false }, // 166
  { "hpmcounter14", 0xC0E,  {} , false }, // 167
  { "hpmcounter15", 0xC0F,  {} , false }, // 168
  { "hpmcounter16", 0xC10,  {} , false }, // 169
  { "hpmcounter17", 0xC11,  {} , false }, // 170
  { "hpmcounter18", 0xC12,  {} , false }, // 171
  { "hpmcounter19", 0xC13,  {} , false }, // 172
  { "hpmcounter20", 0xC14,  {} , false }, // 173
  { "hpmcounter21", 0xC15,  {} , false }, // 174
  { "hpmcounter22", 0xC16,  {} , false }, // 175
  { "hpmcounter23", 0xC17,  {} , false }, // 176
  { "hpmcounter24", 0xC18,  {} , false }, // 177
  { "hpmcounter25", 0xC19,  {} , false }, // 178
  { "hpmcounter26", 0xC1A,  {} , false }, // 179
  { "hpmcounter27", 0xC1B,  {} , false }, // 180
  { "hpmcounter28", 0xC1C,  {} , false }, // 181
  { "hpmcounter29", 0xC1D,  {} , false }, // 182
  { "hpmcounter30", 0xC1E,  {} , false }, // 183
  { "hpmcounter31", 0xC1F,  {} , false }, // 184
  { "cycleh", 0xC80,  {} , true }, // 185
  { "timeh", 0xC81,  {} , true }, // 186
  { "instreth", 0xC82,  {} , true }, // 187
  { "hpmcounter3h", 0xC83,  {} , true }, // 188
  { "hpmcounter4h", 0xC84,  {} , true }, // 189
  { "hpmcounter5h", 0xC85,  {} , true }, // 190
  { "hpmcounter6h", 0xC86,  {} , true }, // 191
  { "hpmcounter7h", 0xC87,  {} , true }, // 192
  { "hpmcounter8h", 0xC88,  {} , true }, // 193
  { "hpmcounter9h", 0xC89,  {} , true }, // 194
  { "hpmcounter10h", 0xC8A,  {} , true }, // 195
  { "hpmcounter11h", 0xC8B,  {} , true }, // 196
  { "hpmcounter12h", 0xC8C,  {} , true }, // 197
  { "hpmcounter13h", 0xC8D,  {} , true }, // 198
  { "hpmcounter14h", 0xC8E,  {} , true }, // 199
  { "hpmcounter15h", 0xC8F,  {} , true }, // 200
  { "hpmcounter16h", 0xC90,  {} , true }, // 201
  { "hpmcounter17h", 0xC91,  {} , true }, // 202
  { "hpmcounter18h", 0xC92,  {} , true }, // 203
  { "hpmcounter19h", 0xC93,  {} , true }, // 204
  { "hpmcounter20h", 0xC94,  {} , true }, // 205
  { "hpmcounter21h", 0xC95,  {} , true }, // 206
  { "hpmcounter22h", 0xC96,  {} , true }, // 207
  { "hpmcounter23h", 0xC97,  {} , true }, // 208
  { "hpmcounter24h", 0xC98,  {} , true }, // 209
  { "hpmcounter25h", 0xC99,  {} , true }, // 210
  { "hpmcounter26h", 0xC9A,  {} , true }, // 211
  { "hpmcounter27h", 0xC9B,  {} , true }, // 212
  { "hpmcounter28h", 0xC9C,  {} , true }, // 213
  { "hpmcounter29h", 0xC9D,  {} , true }, // 214
  { "hpmcounter30h", 0xC9E,  {} , true }, // 215
  { "hpmcounter31h", 0xC9F,  {} , true }, // 216
  { "mvendorid", 0xF11,  {} , false }, // 217
  { "marchid", 0xF12,  {} , false }, // 218
  { "mimpid", 0xF13,  {} , false }, // 219
  { "mhartid", 0xF14,  {} , false }, // 220
 };

const SysReg *lookupSysRegByEncoding(uint16_t Encoding) {
  struct KeyType {
    uint16_t Encoding;
  };
  KeyType Key = { Encoding };
  auto Table = makeArrayRef(SysRegsList);
  auto Idx = std::lower_bound(Table.begin(), Table.end(), Key,
    [](const SysReg &LHS, const KeyType &RHS) {
      if (LHS.Encoding < RHS.Encoding)
        return true;
      if (LHS.Encoding > RHS.Encoding)
        return false;
      return false;
    });

  if (Idx == Table.end() ||
      Key.Encoding != Idx->Encoding)
    return nullptr;
  return &*Idx;
}

const SysReg *lookupSysRegByName(StringRef Name) {
  struct IndexType {
    const char * Name;
    unsigned _index;
  };
  static const struct IndexType Index[] = {
    { "CYCLE", 153 },
    { "CYCLEH", 185 },
    { "DCSR", 88 },
    { "DPC", 89 },
    { "DSCRATCH", 90 },
    { "FCSR", 3 },
    { "FFLAGS", 1 },
    { "FRM", 2 },
    { "HPMCOUNTER10", 163 },
    { "HPMCOUNTER10H", 195 },
    { "HPMCOUNTER11", 164 },
    { "HPMCOUNTER11H", 196 },
    { "HPMCOUNTER12", 165 },
    { "HPMCOUNTER12H", 197 },
    { "HPMCOUNTER13", 166 },
    { "HPMCOUNTER13H", 198 },
    { "HPMCOUNTER14", 167 },
    { "HPMCOUNTER14H", 199 },
    { "HPMCOUNTER15", 168 },
    { "HPMCOUNTER15H", 200 },
    { "HPMCOUNTER16", 169 },
    { "HPMCOUNTER16H", 201 },
    { "HPMCOUNTER17", 170 },
    { "HPMCOUNTER17H", 202 },
    { "HPMCOUNTER18", 171 },
    { "HPMCOUNTER18H", 203 },
    { "HPMCOUNTER19", 172 },
    { "HPMCOUNTER19H", 204 },
    { "HPMCOUNTER20", 173 },
    { "HPMCOUNTER20H", 205 },
    { "HPMCOUNTER21", 174 },
    { "HPMCOUNTER21H", 206 },
    { "HPMCOUNTER22", 175 },
    { "HPMCOUNTER22H", 207 },
    { "HPMCOUNTER23", 176 },
    { "HPMCOUNTER23H", 208 },
    { "HPMCOUNTER24", 177 },
    { "HPMCOUNTER24H", 209 },
    { "HPMCOUNTER25", 178 },
    { "HPMCOUNTER25H", 210 },
    { "HPMCOUNTER26", 179 },
    { "HPMCOUNTER26H", 211 },
    { "HPMCOUNTER27", 180 },
    { "HPMCOUNTER27H", 212 },
    { "HPMCOUNTER28", 181 },
    { "HPMCOUNTER28H", 213 },
    { "HPMCOUNTER29", 182 },
    { "HPMCOUNTER29H", 214 },
    { "HPMCOUNTER3", 156 },
    { "HPMCOUNTER30", 183 },
    { "HPMCOUNTER30H", 215 },
    { "HPMCOUNTER31", 184 },
    { "HPMCOUNTER31H", 216 },
    { "HPMCOUNTER3H", 188 },
    { "HPMCOUNTER4", 157 },
    { "HPMCOUNTER4H", 189 },
    { "HPMCOUNTER5", 158 },
    { "HPMCOUNTER5H", 190 },
    { "HPMCOUNTER6", 159 },
    { "HPMCOUNTER6H", 191 },
    { "HPMCOUNTER7", 160 },
    { "HPMCOUNTER7H", 192 },
    { "HPMCOUNTER8", 161 },
    { "HPMCOUNTER8H", 193 },
    { "HPMCOUNTER9", 162 },
    { "HPMCOUNTER9H", 194 },
    { "INSTRET", 155 },
    { "INSTRETH", 187 },
    { "MARCHID", 218 },
    { "MCAUSE", 61 },
    { "MCOUNTEREN", 29 },
    { "MCYCLE", 91 },
    { "MCYCLEH", 122 },
    { "MEDELEG", 25 },
    { "MEPC", 60 },
    { "MHARTID", 220 },
    { "MHPMCOUNTER10", 100 },
    { "MHPMCOUNTER10H", 131 },
    { "MHPMCOUNTER11", 101 },
    { "MHPMCOUNTER11H", 132 },
    { "MHPMCOUNTER12", 102 },
    { "MHPMCOUNTER12H", 133 },
    { "MHPMCOUNTER13", 103 },
    { "MHPMCOUNTER13H", 134 },
    { "MHPMCOUNTER14", 104 },
    { "MHPMCOUNTER14H", 135 },
    { "MHPMCOUNTER15", 105 },
    { "MHPMCOUNTER15H", 136 },
    { "MHPMCOUNTER16", 106 },
    { "MHPMCOUNTER16H", 137 },
    { "MHPMCOUNTER17", 107 },
    { "MHPMCOUNTER17H", 138 },
    { "MHPMCOUNTER18", 108 },
    { "MHPMCOUNTER18H", 139 },
    { "MHPMCOUNTER19", 109 },
    { "MHPMCOUNTER19H", 140 },
    { "MHPMCOUNTER20", 110 },
    { "MHPMCOUNTER20H", 141 },
    { "MHPMCOUNTER21", 111 },
    { "MHPMCOUNTER21H", 142 },
    { "MHPMCOUNTER22", 112 },
    { "MHPMCOUNTER22H", 143 },
    { "MHPMCOUNTER23", 113 },
    { "MHPMCOUNTER23H", 144 },
    { "MHPMCOUNTER24", 114 },
    { "MHPMCOUNTER24H", 145 },
    { "MHPMCOUNTER25", 115 },
    { "MHPMCOUNTER25H", 146 },
    { "MHPMCOUNTER26", 116 },
    { "MHPMCOUNTER26H", 147 },
    { "MHPMCOUNTER27", 117 },
    { "MHPMCOUNTER27H", 148 },
    { "MHPMCOUNTER28", 118 },
    { "MHPMCOUNTER28H", 149 },
    { "MHPMCOUNTER29", 119 },
    { "MHPMCOUNTER29H", 150 },
    { "MHPMCOUNTER3", 93 },
    { "MHPMCOUNTER30", 120 },
    { "MHPMCOUNTER30H", 151 },
    { "MHPMCOUNTER31", 121 },
    { "MHPMCOUNTER31H", 152 },
    { "MHPMCOUNTER3H", 124 },
    { "MHPMCOUNTER4", 94 },
    { "MHPMCOUNTER4H", 125 },
    { "MHPMCOUNTER5", 95 },
    { "MHPMCOUNTER5H", 126 },
    { "MHPMCOUNTER6", 96 },
    { "MHPMCOUNTER6H", 127 },
    { "MHPMCOUNTER7", 97 },
    { "MHPMCOUNTER7H", 128 },
    { "MHPMCOUNTER8", 98 },
    { "MHPMCOUNTER8H", 129 },
    { "MHPMCOUNTER9", 99 },
    { "MHPMCOUNTER9H", 130 },
    { "MHPMEVENT10", 37 },
    { "MHPMEVENT11", 38 },
    { "MHPMEVENT12", 39 },
    { "MHPMEVENT13", 40 },
    { "MHPMEVENT14", 41 },
    { "MHPMEVENT15", 42 },
    { "MHPMEVENT16", 43 },
    { "MHPMEVENT17", 44 },
    { "MHPMEVENT18", 45 },
    { "MHPMEVENT19", 46 },
    { "MHPMEVENT20", 47 },
    { "MHPMEVENT21", 48 },
    { "MHPMEVENT22", 49 },
    { "MHPMEVENT23", 50 },
    { "MHPMEVENT24", 51 },
    { "MHPMEVENT25", 52 },
    { "MHPMEVENT26", 53 },
    { "MHPMEVENT27", 54 },
    { "MHPMEVENT28", 55 },
    { "MHPMEVENT29", 56 },
    { "MHPMEVENT3", 30 },
    { "MHPMEVENT30", 57 },
    { "MHPMEVENT31", 58 },
    { "MHPMEVENT4", 31 },
    { "MHPMEVENT5", 32 },
    { "MHPMEVENT6", 33 },
    { "MHPMEVENT7", 34 },
    { "MHPMEVENT8", 35 },
    { "MHPMEVENT9", 36 },
    { "MIDELEG", 26 },
    { "MIE", 27 },
    { "MIMPID", 219 },
    { "MINSTRET", 92 },
    { "MINSTRETH", 123 },
    { "MIP", 63 },
    { "MISA", 24 },
    { "MSCRATCH", 59 },
    { "MSTATUS", 23 },
    { "MTVAL", 62 },
    { "MTVEC", 28 },
    { "MVENDORID", 217 },
    { "PMPADDR0", 68 },
    { "PMPADDR1", 69 },
    { "PMPADDR10", 78 },
    { "PMPADDR11", 79 },
    { "PMPADDR12", 80 },
    { "PMPADDR13", 81 },
    { "PMPADDR14", 82 },
    { "PMPADDR15", 83 },
    { "PMPADDR2", 70 },
    { "PMPADDR3", 71 },
    { "PMPADDR4", 72 },
    { "PMPADDR5", 73 },
    { "PMPADDR6", 74 },
    { "PMPADDR7", 75 },
    { "PMPADDR8", 76 },
    { "PMPADDR9", 77 },
    { "PMPCFG0", 64 },
    { "PMPCFG1", 65 },
    { "PMPCFG2", 66 },
    { "PMPCFG3", 67 },
    { "SATP", 22 },
    { "SCAUSE", 19 },
    { "SCOUNTEREN", 16 },
    { "SEDELEG", 12 },
    { "SEPC", 18 },
    { "SIDELEG", 13 },
    { "SIE", 14 },
    { "SIP", 21 },
    { "SSCRATCH", 17 },
    { "SSTATUS", 11 },
    { "STVAL", 20 },
    { "STVEC", 15 },
    { "TDATA1", 85 },
    { "TDATA2", 86 },
    { "TDATA3", 87 },
    { "TIME", 154 },
    { "TIMEH", 186 },
    { "TSELECT", 84 },
    { "UCAUSE", 8 },
    { "UEPC", 7 },
    { "UIE", 4 },
    { "UIP", 10 },
    { "USCRATCH", 6 },
    { "USTATUS", 0 },
    { "UTVAL", 9 },
    { "UTVEC", 5 },
  };

  struct KeyType {
    std::string Name;
  };
  KeyType Key = { Name.upper() };
  auto Table = makeArrayRef(Index);
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
  return &SysRegsList[Idx->_index];
}
#endif

#undef GET_SysRegsList_DECL
#undef GET_SysRegsList_IMPL
