//
// vtx_main
//
struct tint_symbol_1 {
  uint VertexIndex : SV_VertexID;
};
struct tint_symbol_2 {
  float4 value : SV_Position;
};

float4 vtx_main_inner(uint VertexIndex) {
  float2 tint_symbol_3[3] = {float2(0.0f, 0.5f), (-0.5f).xx, float2(0.5f, -0.5f)};
  return float4(tint_symbol_3[min(VertexIndex, 2u)], 0.0f, 1.0f);
}

tint_symbol_2 vtx_main(tint_symbol_1 tint_symbol) {
  float4 inner_result = vtx_main_inner(tint_symbol.VertexIndex);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
//
// frag_main
//
struct tint_symbol {
  float4 value : SV_Target0;
};

float4 frag_main_inner() {
  return float4(1.0f, 0.0f, 0.0f, 1.0f);
}

tint_symbol frag_main() {
  float4 inner_result = frag_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
