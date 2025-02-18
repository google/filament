struct tint_symbol_1 {
  uint in_vertex_index : SV_VertexID;
};
struct tint_symbol_2 {
  float4 value : SV_Position;
};

float4 vs_main_inner(uint in_vertex_index) {
  float4 tint_symbol_3[3] = {float4(0.0f, 0.0f, 0.0f, 1.0f), float4(0.0f, 1.0f, 0.0f, 1.0f), float4(1.0f, 1.0f, 0.0f, 1.0f)};
  return tint_symbol_3[min(in_vertex_index, 2u)];
}

tint_symbol_2 vs_main(tint_symbol_1 tint_symbol) {
  float4 inner_result = vs_main_inner(tint_symbol.in_vertex_index);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
