SKIP: INVALID

struct tint_symbol_1 {
  nointerpolation int loc0 : TEXCOORD0;
  nointerpolation uint loc1 : TEXCOORD1;
  float loc2 : TEXCOORD2;
  float4 loc3 : TEXCOORD3;
  float16_t loc4 : TEXCOORD4;
  vector<float16_t, 3> loc5 : TEXCOORD5;
};

void main_inner(int loc0, uint loc1, float loc2, float4 loc3, float16_t loc4, vector<float16_t, 3> loc5) {
  int i = loc0;
  uint u = loc1;
  float f = loc2;
  float4 v = loc3;
  float16_t x = loc4;
  vector<float16_t, 3> y = loc5;
}

void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.loc0, tint_symbol.loc1, tint_symbol.loc2, tint_symbol.loc3, tint_symbol.loc4, tint_symbol.loc5);
  return;
}
FXC validation failure:
<scrubbed_path>(6,3-11): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
