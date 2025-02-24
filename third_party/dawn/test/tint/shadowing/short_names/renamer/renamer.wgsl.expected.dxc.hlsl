struct tint_symbol_3 {
  uint tint_symbol_1 : SV_VertexID;
};
struct tint_symbol_4 {
  float4 value : SV_Position;
};

float4 tint_symbol_inner(uint tint_symbol_1) {
  return float4(0.0f, 0.0f, 0.0f, 1.0f);
}

tint_symbol_4 tint_symbol(tint_symbol_3 tint_symbol_2) {
  float4 inner_result = tint_symbol_inner(tint_symbol_2.tint_symbol_1);
  tint_symbol_4 wrapper_result = (tint_symbol_4)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
