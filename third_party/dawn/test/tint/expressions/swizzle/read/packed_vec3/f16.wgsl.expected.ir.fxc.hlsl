SKIP: INVALID


cbuffer cbuffer_U : register(b0) {
  uint4 U[1];
};
vector<float16_t, 4> tint_bitcast_to_f16(uint4 src) {
  uint4 v = src;
  uint4 mask = (65535u).xxxx;
  uint4 shift = (16u).xxxx;
  float4 t_low = f16tof32((v & mask));
  float4 t_high = f16tof32(((v >> shift) & mask));
  float16_t v_1 = float16_t(t_low.x);
  float16_t v_2 = float16_t(t_high.x);
  float16_t v_3 = float16_t(t_low.y);
  return vector<float16_t, 4>(v_1, v_2, v_3, float16_t(t_high.y));
}

void f() {
  vector<float16_t, 3> v = tint_bitcast_to_f16(U[0u]).xyz;
  float16_t x = float16_t(f16tof32(U[0u].x));
  float16_t y = float16_t(f16tof32((U[0u].x >> 16u)));
  float16_t z = float16_t(f16tof32(U[0u].y));
  vector<float16_t, 2> xx = tint_bitcast_to_f16(U[0u]).xyz.xx;
  vector<float16_t, 2> xy = tint_bitcast_to_f16(U[0u]).xyz.xy;
  vector<float16_t, 2> xz = tint_bitcast_to_f16(U[0u]).xyz.xz;
  vector<float16_t, 2> yx = tint_bitcast_to_f16(U[0u]).xyz.yx;
  vector<float16_t, 2> yy = tint_bitcast_to_f16(U[0u]).xyz.yy;
  vector<float16_t, 2> yz = tint_bitcast_to_f16(U[0u]).xyz.yz;
  vector<float16_t, 2> zx = tint_bitcast_to_f16(U[0u]).xyz.zx;
  vector<float16_t, 2> zy = tint_bitcast_to_f16(U[0u]).xyz.zy;
  vector<float16_t, 2> zz = tint_bitcast_to_f16(U[0u]).xyz.zz;
  vector<float16_t, 3> xxx = tint_bitcast_to_f16(U[0u]).xyz.xxx;
  vector<float16_t, 3> xxy = tint_bitcast_to_f16(U[0u]).xyz.xxy;
  vector<float16_t, 3> xxz = tint_bitcast_to_f16(U[0u]).xyz.xxz;
  vector<float16_t, 3> xyx = tint_bitcast_to_f16(U[0u]).xyz.xyx;
  vector<float16_t, 3> xyy = tint_bitcast_to_f16(U[0u]).xyz.xyy;
  vector<float16_t, 3> xyz = tint_bitcast_to_f16(U[0u]).xyz.xyz;
  vector<float16_t, 3> xzx = tint_bitcast_to_f16(U[0u]).xyz.xzx;
  vector<float16_t, 3> xzy = tint_bitcast_to_f16(U[0u]).xyz.xzy;
  vector<float16_t, 3> xzz = tint_bitcast_to_f16(U[0u]).xyz.xzz;
  vector<float16_t, 3> yxx = tint_bitcast_to_f16(U[0u]).xyz.yxx;
  vector<float16_t, 3> yxy = tint_bitcast_to_f16(U[0u]).xyz.yxy;
  vector<float16_t, 3> yxz = tint_bitcast_to_f16(U[0u]).xyz.yxz;
  vector<float16_t, 3> yyx = tint_bitcast_to_f16(U[0u]).xyz.yyx;
  vector<float16_t, 3> yyy = tint_bitcast_to_f16(U[0u]).xyz.yyy;
  vector<float16_t, 3> yyz = tint_bitcast_to_f16(U[0u]).xyz.yyz;
  vector<float16_t, 3> yzx = tint_bitcast_to_f16(U[0u]).xyz.yzx;
  vector<float16_t, 3> yzy = tint_bitcast_to_f16(U[0u]).xyz.yzy;
  vector<float16_t, 3> yzz = tint_bitcast_to_f16(U[0u]).xyz.yzz;
  vector<float16_t, 3> zxx = tint_bitcast_to_f16(U[0u]).xyz.zxx;
  vector<float16_t, 3> zxy = tint_bitcast_to_f16(U[0u]).xyz.zxy;
  vector<float16_t, 3> zxz = tint_bitcast_to_f16(U[0u]).xyz.zxz;
  vector<float16_t, 3> zyx = tint_bitcast_to_f16(U[0u]).xyz.zyx;
  vector<float16_t, 3> zyy = tint_bitcast_to_f16(U[0u]).xyz.zyy;
  vector<float16_t, 3> zyz = tint_bitcast_to_f16(U[0u]).xyz.zyz;
  vector<float16_t, 3> zzx = tint_bitcast_to_f16(U[0u]).xyz.zzx;
  vector<float16_t, 3> zzy = tint_bitcast_to_f16(U[0u]).xyz.zzy;
  vector<float16_t, 3> zzz = tint_bitcast_to_f16(U[0u]).xyz.zzz;
  vector<float16_t, 4> xxxx = tint_bitcast_to_f16(U[0u]).xyz.xxxx;
  vector<float16_t, 4> xxxy = tint_bitcast_to_f16(U[0u]).xyz.xxxy;
  vector<float16_t, 4> xxxz = tint_bitcast_to_f16(U[0u]).xyz.xxxz;
  vector<float16_t, 4> xxyx = tint_bitcast_to_f16(U[0u]).xyz.xxyx;
  vector<float16_t, 4> xxyy = tint_bitcast_to_f16(U[0u]).xyz.xxyy;
  vector<float16_t, 4> xxyz = tint_bitcast_to_f16(U[0u]).xyz.xxyz;
  vector<float16_t, 4> xxzx = tint_bitcast_to_f16(U[0u]).xyz.xxzx;
  vector<float16_t, 4> xxzy = tint_bitcast_to_f16(U[0u]).xyz.xxzy;
  vector<float16_t, 4> xxzz = tint_bitcast_to_f16(U[0u]).xyz.xxzz;
  vector<float16_t, 4> xyxx = tint_bitcast_to_f16(U[0u]).xyz.xyxx;
  vector<float16_t, 4> xyxy = tint_bitcast_to_f16(U[0u]).xyz.xyxy;
  vector<float16_t, 4> xyxz = tint_bitcast_to_f16(U[0u]).xyz.xyxz;
  vector<float16_t, 4> xyyx = tint_bitcast_to_f16(U[0u]).xyz.xyyx;
  vector<float16_t, 4> xyyy = tint_bitcast_to_f16(U[0u]).xyz.xyyy;
  vector<float16_t, 4> xyyz = tint_bitcast_to_f16(U[0u]).xyz.xyyz;
  vector<float16_t, 4> xyzx = tint_bitcast_to_f16(U[0u]).xyz.xyzx;
  vector<float16_t, 4> xyzy = tint_bitcast_to_f16(U[0u]).xyz.xyzy;
  vector<float16_t, 4> xyzz = tint_bitcast_to_f16(U[0u]).xyz.xyzz;
  vector<float16_t, 4> xzxx = tint_bitcast_to_f16(U[0u]).xyz.xzxx;
  vector<float16_t, 4> xzxy = tint_bitcast_to_f16(U[0u]).xyz.xzxy;
  vector<float16_t, 4> xzxz = tint_bitcast_to_f16(U[0u]).xyz.xzxz;
  vector<float16_t, 4> xzyx = tint_bitcast_to_f16(U[0u]).xyz.xzyx;
  vector<float16_t, 4> xzyy = tint_bitcast_to_f16(U[0u]).xyz.xzyy;
  vector<float16_t, 4> xzyz = tint_bitcast_to_f16(U[0u]).xyz.xzyz;
  vector<float16_t, 4> xzzx = tint_bitcast_to_f16(U[0u]).xyz.xzzx;
  vector<float16_t, 4> xzzy = tint_bitcast_to_f16(U[0u]).xyz.xzzy;
  vector<float16_t, 4> xzzz = tint_bitcast_to_f16(U[0u]).xyz.xzzz;
  vector<float16_t, 4> yxxx = tint_bitcast_to_f16(U[0u]).xyz.yxxx;
  vector<float16_t, 4> yxxy = tint_bitcast_to_f16(U[0u]).xyz.yxxy;
  vector<float16_t, 4> yxxz = tint_bitcast_to_f16(U[0u]).xyz.yxxz;
  vector<float16_t, 4> yxyx = tint_bitcast_to_f16(U[0u]).xyz.yxyx;
  vector<float16_t, 4> yxyy = tint_bitcast_to_f16(U[0u]).xyz.yxyy;
  vector<float16_t, 4> yxyz = tint_bitcast_to_f16(U[0u]).xyz.yxyz;
  vector<float16_t, 4> yxzx = tint_bitcast_to_f16(U[0u]).xyz.yxzx;
  vector<float16_t, 4> yxzy = tint_bitcast_to_f16(U[0u]).xyz.yxzy;
  vector<float16_t, 4> yxzz = tint_bitcast_to_f16(U[0u]).xyz.yxzz;
  vector<float16_t, 4> yyxx = tint_bitcast_to_f16(U[0u]).xyz.yyxx;
  vector<float16_t, 4> yyxy = tint_bitcast_to_f16(U[0u]).xyz.yyxy;
  vector<float16_t, 4> yyxz = tint_bitcast_to_f16(U[0u]).xyz.yyxz;
  vector<float16_t, 4> yyyx = tint_bitcast_to_f16(U[0u]).xyz.yyyx;
  vector<float16_t, 4> yyyy = tint_bitcast_to_f16(U[0u]).xyz.yyyy;
  vector<float16_t, 4> yyyz = tint_bitcast_to_f16(U[0u]).xyz.yyyz;
  vector<float16_t, 4> yyzx = tint_bitcast_to_f16(U[0u]).xyz.yyzx;
  vector<float16_t, 4> yyzy = tint_bitcast_to_f16(U[0u]).xyz.yyzy;
  vector<float16_t, 4> yyzz = tint_bitcast_to_f16(U[0u]).xyz.yyzz;
  vector<float16_t, 4> yzxx = tint_bitcast_to_f16(U[0u]).xyz.yzxx;
  vector<float16_t, 4> yzxy = tint_bitcast_to_f16(U[0u]).xyz.yzxy;
  vector<float16_t, 4> yzxz = tint_bitcast_to_f16(U[0u]).xyz.yzxz;
  vector<float16_t, 4> yzyx = tint_bitcast_to_f16(U[0u]).xyz.yzyx;
  vector<float16_t, 4> yzyy = tint_bitcast_to_f16(U[0u]).xyz.yzyy;
  vector<float16_t, 4> yzyz = tint_bitcast_to_f16(U[0u]).xyz.yzyz;
  vector<float16_t, 4> yzzx = tint_bitcast_to_f16(U[0u]).xyz.yzzx;
  vector<float16_t, 4> yzzy = tint_bitcast_to_f16(U[0u]).xyz.yzzy;
  vector<float16_t, 4> yzzz = tint_bitcast_to_f16(U[0u]).xyz.yzzz;
  vector<float16_t, 4> zxxx = tint_bitcast_to_f16(U[0u]).xyz.zxxx;
  vector<float16_t, 4> zxxy = tint_bitcast_to_f16(U[0u]).xyz.zxxy;
  vector<float16_t, 4> zxxz = tint_bitcast_to_f16(U[0u]).xyz.zxxz;
  vector<float16_t, 4> zxyx = tint_bitcast_to_f16(U[0u]).xyz.zxyx;
  vector<float16_t, 4> zxyy = tint_bitcast_to_f16(U[0u]).xyz.zxyy;
  vector<float16_t, 4> zxyz = tint_bitcast_to_f16(U[0u]).xyz.zxyz;
  vector<float16_t, 4> zxzx = tint_bitcast_to_f16(U[0u]).xyz.zxzx;
  vector<float16_t, 4> zxzy = tint_bitcast_to_f16(U[0u]).xyz.zxzy;
  vector<float16_t, 4> zxzz = tint_bitcast_to_f16(U[0u]).xyz.zxzz;
  vector<float16_t, 4> zyxx = tint_bitcast_to_f16(U[0u]).xyz.zyxx;
  vector<float16_t, 4> zyxy = tint_bitcast_to_f16(U[0u]).xyz.zyxy;
  vector<float16_t, 4> zyxz = tint_bitcast_to_f16(U[0u]).xyz.zyxz;
  vector<float16_t, 4> zyyx = tint_bitcast_to_f16(U[0u]).xyz.zyyx;
  vector<float16_t, 4> zyyy = tint_bitcast_to_f16(U[0u]).xyz.zyyy;
  vector<float16_t, 4> zyyz = tint_bitcast_to_f16(U[0u]).xyz.zyyz;
  vector<float16_t, 4> zyzx = tint_bitcast_to_f16(U[0u]).xyz.zyzx;
  vector<float16_t, 4> zyzy = tint_bitcast_to_f16(U[0u]).xyz.zyzy;
  vector<float16_t, 4> zyzz = tint_bitcast_to_f16(U[0u]).xyz.zyzz;
  vector<float16_t, 4> zzxx = tint_bitcast_to_f16(U[0u]).xyz.zzxx;
  vector<float16_t, 4> zzxy = tint_bitcast_to_f16(U[0u]).xyz.zzxy;
  vector<float16_t, 4> zzxz = tint_bitcast_to_f16(U[0u]).xyz.zzxz;
  vector<float16_t, 4> zzyx = tint_bitcast_to_f16(U[0u]).xyz.zzyx;
  vector<float16_t, 4> zzyy = tint_bitcast_to_f16(U[0u]).xyz.zzyy;
  vector<float16_t, 4> zzyz = tint_bitcast_to_f16(U[0u]).xyz.zzyz;
  vector<float16_t, 4> zzzx = tint_bitcast_to_f16(U[0u]).xyz.zzzx;
  vector<float16_t, 4> zzzy = tint_bitcast_to_f16(U[0u]).xyz.zzzy;
  vector<float16_t, 4> zzzz = tint_bitcast_to_f16(U[0u]).xyz.zzzz;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
<scrubbed_path>(5,8-16): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
