struct tint_symbol_1 {
  uint VertexIndex : SV_VertexID;
};
struct tint_symbol_2 {
  float4 value : SV_Position;
};

float4 main_inner(uint VertexIndex) {
  return float4(0.0f, 0.0f, 0.0f, 1.0f);
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  float4 inner_result = main_inner(tint_symbol.VertexIndex);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
