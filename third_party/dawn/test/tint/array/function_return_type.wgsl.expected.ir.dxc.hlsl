
RWByteAddressBuffer s : register(u0);
typedef float ary_ret[4];
ary_ret f1() {
  float v[4] = (float[4])0;
  return v;
}

typedef float ary_ret_1[3][4];
ary_ret_1 f2() {
  float v_1[4] = f1();
  float v_2[4] = f1();
  float v_3[4] = f1();
  float v_4[3][4] = {v_1, v_2, v_3};
  return v_4;
}

typedef float ary_ret_2[2][3][4];
ary_ret_2 f3() {
  float v_5[3][4] = f2();
  float v_6[3][4] = f2();
  float v_7[2][3][4] = {v_5, v_6};
  return v_7;
}

[numthreads(1, 1, 1)]
void main() {
  float a1[4] = f1();
  float a2[3][4] = f2();
  float a3[2][3][4] = f3();
  s.Store(0u, asuint(((a1[0u] + a2[0u][0u]) + a3[0u][0u][0u])));
}

