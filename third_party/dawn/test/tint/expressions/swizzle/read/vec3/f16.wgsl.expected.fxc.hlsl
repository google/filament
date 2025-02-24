SKIP: INVALID

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct S {
  vector<float16_t, 3> v;
};

static S P = (S)0;

void f() {
  vector<float16_t, 3> v = P.v;
  float16_t x = P.v.x;
  float16_t y = P.v.y;
  float16_t z = P.v.z;
  vector<float16_t, 2> xx = P.v.xx;
  vector<float16_t, 2> xy = P.v.xy;
  vector<float16_t, 2> xz = P.v.xz;
  vector<float16_t, 2> yx = P.v.yx;
  vector<float16_t, 2> yy = P.v.yy;
  vector<float16_t, 2> yz = P.v.yz;
  vector<float16_t, 2> zx = P.v.zx;
  vector<float16_t, 2> zy = P.v.zy;
  vector<float16_t, 2> zz = P.v.zz;
  vector<float16_t, 3> xxx = P.v.xxx;
  vector<float16_t, 3> xxy = P.v.xxy;
  vector<float16_t, 3> xxz = P.v.xxz;
  vector<float16_t, 3> xyx = P.v.xyx;
  vector<float16_t, 3> xyy = P.v.xyy;
  vector<float16_t, 3> xyz = P.v.xyz;
  vector<float16_t, 3> xzx = P.v.xzx;
  vector<float16_t, 3> xzy = P.v.xzy;
  vector<float16_t, 3> xzz = P.v.xzz;
  vector<float16_t, 3> yxx = P.v.yxx;
  vector<float16_t, 3> yxy = P.v.yxy;
  vector<float16_t, 3> yxz = P.v.yxz;
  vector<float16_t, 3> yyx = P.v.yyx;
  vector<float16_t, 3> yyy = P.v.yyy;
  vector<float16_t, 3> yyz = P.v.yyz;
  vector<float16_t, 3> yzx = P.v.yzx;
  vector<float16_t, 3> yzy = P.v.yzy;
  vector<float16_t, 3> yzz = P.v.yzz;
  vector<float16_t, 3> zxx = P.v.zxx;
  vector<float16_t, 3> zxy = P.v.zxy;
  vector<float16_t, 3> zxz = P.v.zxz;
  vector<float16_t, 3> zyx = P.v.zyx;
  vector<float16_t, 3> zyy = P.v.zyy;
  vector<float16_t, 3> zyz = P.v.zyz;
  vector<float16_t, 3> zzx = P.v.zzx;
  vector<float16_t, 3> zzy = P.v.zzy;
  vector<float16_t, 3> zzz = P.v.zzz;
  vector<float16_t, 4> xxxx = P.v.xxxx;
  vector<float16_t, 4> xxxy = P.v.xxxy;
  vector<float16_t, 4> xxxz = P.v.xxxz;
  vector<float16_t, 4> xxyx = P.v.xxyx;
  vector<float16_t, 4> xxyy = P.v.xxyy;
  vector<float16_t, 4> xxyz = P.v.xxyz;
  vector<float16_t, 4> xxzx = P.v.xxzx;
  vector<float16_t, 4> xxzy = P.v.xxzy;
  vector<float16_t, 4> xxzz = P.v.xxzz;
  vector<float16_t, 4> xyxx = P.v.xyxx;
  vector<float16_t, 4> xyxy = P.v.xyxy;
  vector<float16_t, 4> xyxz = P.v.xyxz;
  vector<float16_t, 4> xyyx = P.v.xyyx;
  vector<float16_t, 4> xyyy = P.v.xyyy;
  vector<float16_t, 4> xyyz = P.v.xyyz;
  vector<float16_t, 4> xyzx = P.v.xyzx;
  vector<float16_t, 4> xyzy = P.v.xyzy;
  vector<float16_t, 4> xyzz = P.v.xyzz;
  vector<float16_t, 4> xzxx = P.v.xzxx;
  vector<float16_t, 4> xzxy = P.v.xzxy;
  vector<float16_t, 4> xzxz = P.v.xzxz;
  vector<float16_t, 4> xzyx = P.v.xzyx;
  vector<float16_t, 4> xzyy = P.v.xzyy;
  vector<float16_t, 4> xzyz = P.v.xzyz;
  vector<float16_t, 4> xzzx = P.v.xzzx;
  vector<float16_t, 4> xzzy = P.v.xzzy;
  vector<float16_t, 4> xzzz = P.v.xzzz;
  vector<float16_t, 4> yxxx = P.v.yxxx;
  vector<float16_t, 4> yxxy = P.v.yxxy;
  vector<float16_t, 4> yxxz = P.v.yxxz;
  vector<float16_t, 4> yxyx = P.v.yxyx;
  vector<float16_t, 4> yxyy = P.v.yxyy;
  vector<float16_t, 4> yxyz = P.v.yxyz;
  vector<float16_t, 4> yxzx = P.v.yxzx;
  vector<float16_t, 4> yxzy = P.v.yxzy;
  vector<float16_t, 4> yxzz = P.v.yxzz;
  vector<float16_t, 4> yyxx = P.v.yyxx;
  vector<float16_t, 4> yyxy = P.v.yyxy;
  vector<float16_t, 4> yyxz = P.v.yyxz;
  vector<float16_t, 4> yyyx = P.v.yyyx;
  vector<float16_t, 4> yyyy = P.v.yyyy;
  vector<float16_t, 4> yyyz = P.v.yyyz;
  vector<float16_t, 4> yyzx = P.v.yyzx;
  vector<float16_t, 4> yyzy = P.v.yyzy;
  vector<float16_t, 4> yyzz = P.v.yyzz;
  vector<float16_t, 4> yzxx = P.v.yzxx;
  vector<float16_t, 4> yzxy = P.v.yzxy;
  vector<float16_t, 4> yzxz = P.v.yzxz;
  vector<float16_t, 4> yzyx = P.v.yzyx;
  vector<float16_t, 4> yzyy = P.v.yzyy;
  vector<float16_t, 4> yzyz = P.v.yzyz;
  vector<float16_t, 4> yzzx = P.v.yzzx;
  vector<float16_t, 4> yzzy = P.v.yzzy;
  vector<float16_t, 4> yzzz = P.v.yzzz;
  vector<float16_t, 4> zxxx = P.v.zxxx;
  vector<float16_t, 4> zxxy = P.v.zxxy;
  vector<float16_t, 4> zxxz = P.v.zxxz;
  vector<float16_t, 4> zxyx = P.v.zxyx;
  vector<float16_t, 4> zxyy = P.v.zxyy;
  vector<float16_t, 4> zxyz = P.v.zxyz;
  vector<float16_t, 4> zxzx = P.v.zxzx;
  vector<float16_t, 4> zxzy = P.v.zxzy;
  vector<float16_t, 4> zxzz = P.v.zxzz;
  vector<float16_t, 4> zyxx = P.v.zyxx;
  vector<float16_t, 4> zyxy = P.v.zyxy;
  vector<float16_t, 4> zyxz = P.v.zyxz;
  vector<float16_t, 4> zyyx = P.v.zyyx;
  vector<float16_t, 4> zyyy = P.v.zyyy;
  vector<float16_t, 4> zyyz = P.v.zyyz;
  vector<float16_t, 4> zyzx = P.v.zyzx;
  vector<float16_t, 4> zyzy = P.v.zyzy;
  vector<float16_t, 4> zyzz = P.v.zyzz;
  vector<float16_t, 4> zzxx = P.v.zzxx;
  vector<float16_t, 4> zzxy = P.v.zzxy;
  vector<float16_t, 4> zzxz = P.v.zzxz;
  vector<float16_t, 4> zzyx = P.v.zzyx;
  vector<float16_t, 4> zzyy = P.v.zzyy;
  vector<float16_t, 4> zzyz = P.v.zzyz;
  vector<float16_t, 4> zzzx = P.v.zzzx;
  vector<float16_t, 4> zzzy = P.v.zzzy;
  vector<float16_t, 4> zzzz = P.v.zzzz;
}
FXC validation failure:
<scrubbed_path>(7,10-18): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
