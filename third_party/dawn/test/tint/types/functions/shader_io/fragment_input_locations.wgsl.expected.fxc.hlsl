struct tint_symbol_1 {
  nointerpolation int loc0 : TEXCOORD0;
  nointerpolation uint loc1 : TEXCOORD1;
  float loc2 : TEXCOORD2;
  float4 loc3 : TEXCOORD3;
};

void main_inner(int loc0, uint loc1, float loc2, float4 loc3) {
  int i = loc0;
  uint u = loc1;
  float f = loc2;
  float4 v = loc3;
}

void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.loc0, tint_symbol.loc1, tint_symbol.loc2, tint_symbol.loc3);
  return;
}
