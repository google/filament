struct tint_symbol {
  int tint_symbol_1;
};
struct tint_symbol_10 {
  uint tint_symbol_5 : SV_VertexID;
};
struct tint_symbol_11 {
  float4 value : SV_Position;
};

float4 tint_symbol_4_inner(uint tint_symbol_5) {
  tint_symbol tint_symbol_6 = {1};
  float tint_symbol_7 = float(tint_symbol_6.tint_symbol_1);
  bool tint_symbol_8 = bool(tint_symbol_7);
  return (tint_symbol_8 ? (1.0f).xxxx : (0.0f).xxxx);
}

tint_symbol_11 tint_symbol_4(tint_symbol_10 tint_symbol_9) {
  float4 inner_result = tint_symbol_4_inner(tint_symbol_9.tint_symbol_5);
  tint_symbol_11 wrapper_result = (tint_symbol_11)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
