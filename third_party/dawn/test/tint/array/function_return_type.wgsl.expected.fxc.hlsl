RWByteAddressBuffer s : register(u0);

typedef float f1_ret[4];
f1_ret f1() {
  float tint_symbol_5[4] = (float[4])0;
  return tint_symbol_5;
}

typedef float f2_ret[3][4];
f2_ret f2() {
  float tint_symbol[4] = f1();
  float tint_symbol_1[4] = f1();
  float tint_symbol_2[4] = f1();
  float tint_symbol_6[3][4] = {tint_symbol, tint_symbol_1, tint_symbol_2};
  return tint_symbol_6;
}

typedef float f3_ret[2][3][4];
f3_ret f3() {
  float tint_symbol_3[3][4] = f2();
  float tint_symbol_4[3][4] = f2();
  float tint_symbol_7[2][3][4] = {tint_symbol_3, tint_symbol_4};
  return tint_symbol_7;
}

[numthreads(1, 1, 1)]
void main() {
  float a1[4] = f1();
  float a2[3][4] = f2();
  float a3[2][3][4] = f3();
  s.Store(0u, asuint(((a1[0] + a2[0][0]) + a3[0][0][0])));
  return;
}
