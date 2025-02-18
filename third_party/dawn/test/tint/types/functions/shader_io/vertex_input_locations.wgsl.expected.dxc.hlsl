struct tint_symbol_1 {
  int loc0 : TEXCOORD0;
  uint loc1 : TEXCOORD1;
  float loc2 : TEXCOORD2;
  float4 loc3 : TEXCOORD3;
};
struct tint_symbol_2 {
  float4 value : SV_Position;
};

float4 main_inner(int loc0, uint loc1, float loc2, float4 loc3) {
  int i = loc0;
  uint u = loc1;
  float f = loc2;
  float4 v = loc3;
  return (0.0f).xxxx;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  float4 inner_result = main_inner(tint_symbol.loc0, tint_symbol.loc1, tint_symbol.loc2, tint_symbol.loc3);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
