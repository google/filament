#ifdef GET_AMDGPUImageDMaskIntrinsicTable_DECL
const AMDGPUImageDMaskIntrinsic *getAMDGPUImageDMaskIntrinsic(unsigned Intr);
#endif

#ifdef GET_AMDGPUImageDMaskIntrinsicTable_IMPL
constexpr AMDGPUImageDMaskIntrinsic AMDGPUImageDMaskIntrinsicTable[] = {
  { Intrinsic::amdgcn_image_getlod_1d }, // 0
  { Intrinsic::amdgcn_image_getlod_1darray }, // 1
  { Intrinsic::amdgcn_image_getlod_2d }, // 2
  { Intrinsic::amdgcn_image_getlod_2darray }, // 3
  { Intrinsic::amdgcn_image_getlod_3d }, // 4
  { Intrinsic::amdgcn_image_getlod_cube }, // 5
  { Intrinsic::amdgcn_image_getresinfo_1d }, // 6
  { Intrinsic::amdgcn_image_getresinfo_1darray }, // 7
  { Intrinsic::amdgcn_image_getresinfo_2d }, // 8
  { Intrinsic::amdgcn_image_getresinfo_2darray }, // 9
  { Intrinsic::amdgcn_image_getresinfo_2darraymsaa }, // 10
  { Intrinsic::amdgcn_image_getresinfo_2dmsaa }, // 11
  { Intrinsic::amdgcn_image_getresinfo_3d }, // 12
  { Intrinsic::amdgcn_image_getresinfo_cube }, // 13
  { Intrinsic::amdgcn_image_load_1d }, // 14
  { Intrinsic::amdgcn_image_load_1darray }, // 15
  { Intrinsic::amdgcn_image_load_2d }, // 16
  { Intrinsic::amdgcn_image_load_2darray }, // 17
  { Intrinsic::amdgcn_image_load_2darraymsaa }, // 18
  { Intrinsic::amdgcn_image_load_2dmsaa }, // 19
  { Intrinsic::amdgcn_image_load_3d }, // 20
  { Intrinsic::amdgcn_image_load_cube }, // 21
  { Intrinsic::amdgcn_image_load_mip_1d }, // 22
  { Intrinsic::amdgcn_image_load_mip_1darray }, // 23
  { Intrinsic::amdgcn_image_load_mip_2d }, // 24
  { Intrinsic::amdgcn_image_load_mip_2darray }, // 25
  { Intrinsic::amdgcn_image_load_mip_3d }, // 26
  { Intrinsic::amdgcn_image_load_mip_cube }, // 27
  { Intrinsic::amdgcn_image_sample_1d }, // 28
  { Intrinsic::amdgcn_image_sample_1darray }, // 29
  { Intrinsic::amdgcn_image_sample_2d }, // 30
  { Intrinsic::amdgcn_image_sample_2darray }, // 31
  { Intrinsic::amdgcn_image_sample_3d }, // 32
  { Intrinsic::amdgcn_image_sample_b_1d }, // 33
  { Intrinsic::amdgcn_image_sample_b_1darray }, // 34
  { Intrinsic::amdgcn_image_sample_b_2d }, // 35
  { Intrinsic::amdgcn_image_sample_b_2darray }, // 36
  { Intrinsic::amdgcn_image_sample_b_3d }, // 37
  { Intrinsic::amdgcn_image_sample_b_cl_1d }, // 38
  { Intrinsic::amdgcn_image_sample_b_cl_1darray }, // 39
  { Intrinsic::amdgcn_image_sample_b_cl_2d }, // 40
  { Intrinsic::amdgcn_image_sample_b_cl_2darray }, // 41
  { Intrinsic::amdgcn_image_sample_b_cl_3d }, // 42
  { Intrinsic::amdgcn_image_sample_b_cl_cube }, // 43
  { Intrinsic::amdgcn_image_sample_b_cl_o_1d }, // 44
  { Intrinsic::amdgcn_image_sample_b_cl_o_1darray }, // 45
  { Intrinsic::amdgcn_image_sample_b_cl_o_2d }, // 46
  { Intrinsic::amdgcn_image_sample_b_cl_o_2darray }, // 47
  { Intrinsic::amdgcn_image_sample_b_cl_o_3d }, // 48
  { Intrinsic::amdgcn_image_sample_b_cl_o_cube }, // 49
  { Intrinsic::amdgcn_image_sample_b_cube }, // 50
  { Intrinsic::amdgcn_image_sample_b_o_1d }, // 51
  { Intrinsic::amdgcn_image_sample_b_o_1darray }, // 52
  { Intrinsic::amdgcn_image_sample_b_o_2d }, // 53
  { Intrinsic::amdgcn_image_sample_b_o_2darray }, // 54
  { Intrinsic::amdgcn_image_sample_b_o_3d }, // 55
  { Intrinsic::amdgcn_image_sample_b_o_cube }, // 56
  { Intrinsic::amdgcn_image_sample_c_1d }, // 57
  { Intrinsic::amdgcn_image_sample_c_1darray }, // 58
  { Intrinsic::amdgcn_image_sample_c_2d }, // 59
  { Intrinsic::amdgcn_image_sample_c_2darray }, // 60
  { Intrinsic::amdgcn_image_sample_c_3d }, // 61
  { Intrinsic::amdgcn_image_sample_c_b_1d }, // 62
  { Intrinsic::amdgcn_image_sample_c_b_1darray }, // 63
  { Intrinsic::amdgcn_image_sample_c_b_2d }, // 64
  { Intrinsic::amdgcn_image_sample_c_b_2darray }, // 65
  { Intrinsic::amdgcn_image_sample_c_b_3d }, // 66
  { Intrinsic::amdgcn_image_sample_c_b_cl_1d }, // 67
  { Intrinsic::amdgcn_image_sample_c_b_cl_1darray }, // 68
  { Intrinsic::amdgcn_image_sample_c_b_cl_2d }, // 69
  { Intrinsic::amdgcn_image_sample_c_b_cl_2darray }, // 70
  { Intrinsic::amdgcn_image_sample_c_b_cl_3d }, // 71
  { Intrinsic::amdgcn_image_sample_c_b_cl_cube }, // 72
  { Intrinsic::amdgcn_image_sample_c_b_cl_o_1d }, // 73
  { Intrinsic::amdgcn_image_sample_c_b_cl_o_1darray }, // 74
  { Intrinsic::amdgcn_image_sample_c_b_cl_o_2d }, // 75
  { Intrinsic::amdgcn_image_sample_c_b_cl_o_2darray }, // 76
  { Intrinsic::amdgcn_image_sample_c_b_cl_o_3d }, // 77
  { Intrinsic::amdgcn_image_sample_c_b_cl_o_cube }, // 78
  { Intrinsic::amdgcn_image_sample_c_b_cube }, // 79
  { Intrinsic::amdgcn_image_sample_c_b_o_1d }, // 80
  { Intrinsic::amdgcn_image_sample_c_b_o_1darray }, // 81
  { Intrinsic::amdgcn_image_sample_c_b_o_2d }, // 82
  { Intrinsic::amdgcn_image_sample_c_b_o_2darray }, // 83
  { Intrinsic::amdgcn_image_sample_c_b_o_3d }, // 84
  { Intrinsic::amdgcn_image_sample_c_b_o_cube }, // 85
  { Intrinsic::amdgcn_image_sample_c_cd_1d }, // 86
  { Intrinsic::amdgcn_image_sample_c_cd_1darray }, // 87
  { Intrinsic::amdgcn_image_sample_c_cd_2d }, // 88
  { Intrinsic::amdgcn_image_sample_c_cd_2darray }, // 89
  { Intrinsic::amdgcn_image_sample_c_cd_3d }, // 90
  { Intrinsic::amdgcn_image_sample_c_cd_cl_1d }, // 91
  { Intrinsic::amdgcn_image_sample_c_cd_cl_1darray }, // 92
  { Intrinsic::amdgcn_image_sample_c_cd_cl_2d }, // 93
  { Intrinsic::amdgcn_image_sample_c_cd_cl_2darray }, // 94
  { Intrinsic::amdgcn_image_sample_c_cd_cl_3d }, // 95
  { Intrinsic::amdgcn_image_sample_c_cd_cl_cube }, // 96
  { Intrinsic::amdgcn_image_sample_c_cd_cl_o_1d }, // 97
  { Intrinsic::amdgcn_image_sample_c_cd_cl_o_1darray }, // 98
  { Intrinsic::amdgcn_image_sample_c_cd_cl_o_2d }, // 99
  { Intrinsic::amdgcn_image_sample_c_cd_cl_o_2darray }, // 100
  { Intrinsic::amdgcn_image_sample_c_cd_cl_o_3d }, // 101
  { Intrinsic::amdgcn_image_sample_c_cd_cl_o_cube }, // 102
  { Intrinsic::amdgcn_image_sample_c_cd_cube }, // 103
  { Intrinsic::amdgcn_image_sample_c_cd_o_1d }, // 104
  { Intrinsic::amdgcn_image_sample_c_cd_o_1darray }, // 105
  { Intrinsic::amdgcn_image_sample_c_cd_o_2d }, // 106
  { Intrinsic::amdgcn_image_sample_c_cd_o_2darray }, // 107
  { Intrinsic::amdgcn_image_sample_c_cd_o_3d }, // 108
  { Intrinsic::amdgcn_image_sample_c_cd_o_cube }, // 109
  { Intrinsic::amdgcn_image_sample_c_cl_1d }, // 110
  { Intrinsic::amdgcn_image_sample_c_cl_1darray }, // 111
  { Intrinsic::amdgcn_image_sample_c_cl_2d }, // 112
  { Intrinsic::amdgcn_image_sample_c_cl_2darray }, // 113
  { Intrinsic::amdgcn_image_sample_c_cl_3d }, // 114
  { Intrinsic::amdgcn_image_sample_c_cl_cube }, // 115
  { Intrinsic::amdgcn_image_sample_c_cl_o_1d }, // 116
  { Intrinsic::amdgcn_image_sample_c_cl_o_1darray }, // 117
  { Intrinsic::amdgcn_image_sample_c_cl_o_2d }, // 118
  { Intrinsic::amdgcn_image_sample_c_cl_o_2darray }, // 119
  { Intrinsic::amdgcn_image_sample_c_cl_o_3d }, // 120
  { Intrinsic::amdgcn_image_sample_c_cl_o_cube }, // 121
  { Intrinsic::amdgcn_image_sample_c_cube }, // 122
  { Intrinsic::amdgcn_image_sample_c_d_1d }, // 123
  { Intrinsic::amdgcn_image_sample_c_d_1darray }, // 124
  { Intrinsic::amdgcn_image_sample_c_d_2d }, // 125
  { Intrinsic::amdgcn_image_sample_c_d_2darray }, // 126
  { Intrinsic::amdgcn_image_sample_c_d_3d }, // 127
  { Intrinsic::amdgcn_image_sample_c_d_cl_1d }, // 128
  { Intrinsic::amdgcn_image_sample_c_d_cl_1darray }, // 129
  { Intrinsic::amdgcn_image_sample_c_d_cl_2d }, // 130
  { Intrinsic::amdgcn_image_sample_c_d_cl_2darray }, // 131
  { Intrinsic::amdgcn_image_sample_c_d_cl_3d }, // 132
  { Intrinsic::amdgcn_image_sample_c_d_cl_cube }, // 133
  { Intrinsic::amdgcn_image_sample_c_d_cl_o_1d }, // 134
  { Intrinsic::amdgcn_image_sample_c_d_cl_o_1darray }, // 135
  { Intrinsic::amdgcn_image_sample_c_d_cl_o_2d }, // 136
  { Intrinsic::amdgcn_image_sample_c_d_cl_o_2darray }, // 137
  { Intrinsic::amdgcn_image_sample_c_d_cl_o_3d }, // 138
  { Intrinsic::amdgcn_image_sample_c_d_cl_o_cube }, // 139
  { Intrinsic::amdgcn_image_sample_c_d_cube }, // 140
  { Intrinsic::amdgcn_image_sample_c_d_o_1d }, // 141
  { Intrinsic::amdgcn_image_sample_c_d_o_1darray }, // 142
  { Intrinsic::amdgcn_image_sample_c_d_o_2d }, // 143
  { Intrinsic::amdgcn_image_sample_c_d_o_2darray }, // 144
  { Intrinsic::amdgcn_image_sample_c_d_o_3d }, // 145
  { Intrinsic::amdgcn_image_sample_c_d_o_cube }, // 146
  { Intrinsic::amdgcn_image_sample_c_l_1d }, // 147
  { Intrinsic::amdgcn_image_sample_c_l_1darray }, // 148
  { Intrinsic::amdgcn_image_sample_c_l_2d }, // 149
  { Intrinsic::amdgcn_image_sample_c_l_2darray }, // 150
  { Intrinsic::amdgcn_image_sample_c_l_3d }, // 151
  { Intrinsic::amdgcn_image_sample_c_l_cube }, // 152
  { Intrinsic::amdgcn_image_sample_c_l_o_1d }, // 153
  { Intrinsic::amdgcn_image_sample_c_l_o_1darray }, // 154
  { Intrinsic::amdgcn_image_sample_c_l_o_2d }, // 155
  { Intrinsic::amdgcn_image_sample_c_l_o_2darray }, // 156
  { Intrinsic::amdgcn_image_sample_c_l_o_3d }, // 157
  { Intrinsic::amdgcn_image_sample_c_l_o_cube }, // 158
  { Intrinsic::amdgcn_image_sample_c_lz_1d }, // 159
  { Intrinsic::amdgcn_image_sample_c_lz_1darray }, // 160
  { Intrinsic::amdgcn_image_sample_c_lz_2d }, // 161
  { Intrinsic::amdgcn_image_sample_c_lz_2darray }, // 162
  { Intrinsic::amdgcn_image_sample_c_lz_3d }, // 163
  { Intrinsic::amdgcn_image_sample_c_lz_cube }, // 164
  { Intrinsic::amdgcn_image_sample_c_lz_o_1d }, // 165
  { Intrinsic::amdgcn_image_sample_c_lz_o_1darray }, // 166
  { Intrinsic::amdgcn_image_sample_c_lz_o_2d }, // 167
  { Intrinsic::amdgcn_image_sample_c_lz_o_2darray }, // 168
  { Intrinsic::amdgcn_image_sample_c_lz_o_3d }, // 169
  { Intrinsic::amdgcn_image_sample_c_lz_o_cube }, // 170
  { Intrinsic::amdgcn_image_sample_c_o_1d }, // 171
  { Intrinsic::amdgcn_image_sample_c_o_1darray }, // 172
  { Intrinsic::amdgcn_image_sample_c_o_2d }, // 173
  { Intrinsic::amdgcn_image_sample_c_o_2darray }, // 174
  { Intrinsic::amdgcn_image_sample_c_o_3d }, // 175
  { Intrinsic::amdgcn_image_sample_c_o_cube }, // 176
  { Intrinsic::amdgcn_image_sample_cd_1d }, // 177
  { Intrinsic::amdgcn_image_sample_cd_1darray }, // 178
  { Intrinsic::amdgcn_image_sample_cd_2d }, // 179
  { Intrinsic::amdgcn_image_sample_cd_2darray }, // 180
  { Intrinsic::amdgcn_image_sample_cd_3d }, // 181
  { Intrinsic::amdgcn_image_sample_cd_cl_1d }, // 182
  { Intrinsic::amdgcn_image_sample_cd_cl_1darray }, // 183
  { Intrinsic::amdgcn_image_sample_cd_cl_2d }, // 184
  { Intrinsic::amdgcn_image_sample_cd_cl_2darray }, // 185
  { Intrinsic::amdgcn_image_sample_cd_cl_3d }, // 186
  { Intrinsic::amdgcn_image_sample_cd_cl_cube }, // 187
  { Intrinsic::amdgcn_image_sample_cd_cl_o_1d }, // 188
  { Intrinsic::amdgcn_image_sample_cd_cl_o_1darray }, // 189
  { Intrinsic::amdgcn_image_sample_cd_cl_o_2d }, // 190
  { Intrinsic::amdgcn_image_sample_cd_cl_o_2darray }, // 191
  { Intrinsic::amdgcn_image_sample_cd_cl_o_3d }, // 192
  { Intrinsic::amdgcn_image_sample_cd_cl_o_cube }, // 193
  { Intrinsic::amdgcn_image_sample_cd_cube }, // 194
  { Intrinsic::amdgcn_image_sample_cd_o_1d }, // 195
  { Intrinsic::amdgcn_image_sample_cd_o_1darray }, // 196
  { Intrinsic::amdgcn_image_sample_cd_o_2d }, // 197
  { Intrinsic::amdgcn_image_sample_cd_o_2darray }, // 198
  { Intrinsic::amdgcn_image_sample_cd_o_3d }, // 199
  { Intrinsic::amdgcn_image_sample_cd_o_cube }, // 200
  { Intrinsic::amdgcn_image_sample_cl_1d }, // 201
  { Intrinsic::amdgcn_image_sample_cl_1darray }, // 202
  { Intrinsic::amdgcn_image_sample_cl_2d }, // 203
  { Intrinsic::amdgcn_image_sample_cl_2darray }, // 204
  { Intrinsic::amdgcn_image_sample_cl_3d }, // 205
  { Intrinsic::amdgcn_image_sample_cl_cube }, // 206
  { Intrinsic::amdgcn_image_sample_cl_o_1d }, // 207
  { Intrinsic::amdgcn_image_sample_cl_o_1darray }, // 208
  { Intrinsic::amdgcn_image_sample_cl_o_2d }, // 209
  { Intrinsic::amdgcn_image_sample_cl_o_2darray }, // 210
  { Intrinsic::amdgcn_image_sample_cl_o_3d }, // 211
  { Intrinsic::amdgcn_image_sample_cl_o_cube }, // 212
  { Intrinsic::amdgcn_image_sample_cube }, // 213
  { Intrinsic::amdgcn_image_sample_d_1d }, // 214
  { Intrinsic::amdgcn_image_sample_d_1darray }, // 215
  { Intrinsic::amdgcn_image_sample_d_2d }, // 216
  { Intrinsic::amdgcn_image_sample_d_2darray }, // 217
  { Intrinsic::amdgcn_image_sample_d_3d }, // 218
  { Intrinsic::amdgcn_image_sample_d_cl_1d }, // 219
  { Intrinsic::amdgcn_image_sample_d_cl_1darray }, // 220
  { Intrinsic::amdgcn_image_sample_d_cl_2d }, // 221
  { Intrinsic::amdgcn_image_sample_d_cl_2darray }, // 222
  { Intrinsic::amdgcn_image_sample_d_cl_3d }, // 223
  { Intrinsic::amdgcn_image_sample_d_cl_cube }, // 224
  { Intrinsic::amdgcn_image_sample_d_cl_o_1d }, // 225
  { Intrinsic::amdgcn_image_sample_d_cl_o_1darray }, // 226
  { Intrinsic::amdgcn_image_sample_d_cl_o_2d }, // 227
  { Intrinsic::amdgcn_image_sample_d_cl_o_2darray }, // 228
  { Intrinsic::amdgcn_image_sample_d_cl_o_3d }, // 229
  { Intrinsic::amdgcn_image_sample_d_cl_o_cube }, // 230
  { Intrinsic::amdgcn_image_sample_d_cube }, // 231
  { Intrinsic::amdgcn_image_sample_d_o_1d }, // 232
  { Intrinsic::amdgcn_image_sample_d_o_1darray }, // 233
  { Intrinsic::amdgcn_image_sample_d_o_2d }, // 234
  { Intrinsic::amdgcn_image_sample_d_o_2darray }, // 235
  { Intrinsic::amdgcn_image_sample_d_o_3d }, // 236
  { Intrinsic::amdgcn_image_sample_d_o_cube }, // 237
  { Intrinsic::amdgcn_image_sample_l_1d }, // 238
  { Intrinsic::amdgcn_image_sample_l_1darray }, // 239
  { Intrinsic::amdgcn_image_sample_l_2d }, // 240
  { Intrinsic::amdgcn_image_sample_l_2darray }, // 241
  { Intrinsic::amdgcn_image_sample_l_3d }, // 242
  { Intrinsic::amdgcn_image_sample_l_cube }, // 243
  { Intrinsic::amdgcn_image_sample_l_o_1d }, // 244
  { Intrinsic::amdgcn_image_sample_l_o_1darray }, // 245
  { Intrinsic::amdgcn_image_sample_l_o_2d }, // 246
  { Intrinsic::amdgcn_image_sample_l_o_2darray }, // 247
  { Intrinsic::amdgcn_image_sample_l_o_3d }, // 248
  { Intrinsic::amdgcn_image_sample_l_o_cube }, // 249
  { Intrinsic::amdgcn_image_sample_lz_1d }, // 250
  { Intrinsic::amdgcn_image_sample_lz_1darray }, // 251
  { Intrinsic::amdgcn_image_sample_lz_2d }, // 252
  { Intrinsic::amdgcn_image_sample_lz_2darray }, // 253
  { Intrinsic::amdgcn_image_sample_lz_3d }, // 254
  { Intrinsic::amdgcn_image_sample_lz_cube }, // 255
  { Intrinsic::amdgcn_image_sample_lz_o_1d }, // 256
  { Intrinsic::amdgcn_image_sample_lz_o_1darray }, // 257
  { Intrinsic::amdgcn_image_sample_lz_o_2d }, // 258
  { Intrinsic::amdgcn_image_sample_lz_o_2darray }, // 259
  { Intrinsic::amdgcn_image_sample_lz_o_3d }, // 260
  { Intrinsic::amdgcn_image_sample_lz_o_cube }, // 261
  { Intrinsic::amdgcn_image_sample_o_1d }, // 262
  { Intrinsic::amdgcn_image_sample_o_1darray }, // 263
  { Intrinsic::amdgcn_image_sample_o_2d }, // 264
  { Intrinsic::amdgcn_image_sample_o_2darray }, // 265
  { Intrinsic::amdgcn_image_sample_o_3d }, // 266
  { Intrinsic::amdgcn_image_sample_o_cube }, // 267
 };

const AMDGPUImageDMaskIntrinsic *getAMDGPUImageDMaskIntrinsic(unsigned Intr) {
  if ((Intr < Intrinsic::amdgcn_image_getlod_1d) ||
      (Intr > Intrinsic::amdgcn_image_sample_o_cube))
    return nullptr;

  struct KeyType {
    unsigned Intr;
  };
  KeyType Key = { Intr };
  auto Table = makeArrayRef(AMDGPUImageDMaskIntrinsicTable);
  auto Idx = std::lower_bound(Table.begin(), Table.end(), Key,
    [](const AMDGPUImageDMaskIntrinsic &LHS, const KeyType &RHS) {
      if (LHS.Intr < RHS.Intr)
        return true;
      if (LHS.Intr > RHS.Intr)
        return false;
      return false;
    });

  if (Idx == Table.end() ||
      Key.Intr != Idx->Intr)
    return nullptr;
  return &*Idx;
}
#endif

#undef GET_AMDGPUImageDMaskIntrinsicTable_DECL
#undef GET_AMDGPUImageDMaskIntrinsicTable_IMPL
