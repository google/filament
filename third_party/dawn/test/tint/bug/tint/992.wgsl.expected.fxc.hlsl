struct tint_symbol {
  float4 value : SV_Target0;
};

float4 frag_main_inner() {
  float b = 0.0f;
  float3 v = float3((b).xxx);
  return float4(v, 1.0f);
}

tint_symbol frag_main() {
  float4 inner_result = frag_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
