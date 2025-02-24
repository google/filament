struct FragIn {
  float a;
  uint mask;
};
struct tint_symbol_2 {
  float a : TEXCOORD0;
  float b : TEXCOORD1;
  uint mask : SV_Coverage;
};
struct tint_symbol_3 {
  float a : SV_Target0;
  uint mask : SV_Coverage;
};

FragIn main_inner(FragIn tint_symbol, float b) {
  if ((tint_symbol.mask == 0u)) {
    return tint_symbol;
  }
  FragIn tint_symbol_5 = {b, 1u};
  return tint_symbol_5;
}

tint_symbol_3 main(tint_symbol_2 tint_symbol_1) {
  FragIn tint_symbol_4 = {tint_symbol_1.a, tint_symbol_1.mask};
  FragIn inner_result = main_inner(tint_symbol_4, tint_symbol_1.b);
  tint_symbol_3 wrapper_result = (tint_symbol_3)0;
  wrapper_result.a = inner_result.a;
  wrapper_result.mask = inner_result.mask;
  return wrapper_result;
}
