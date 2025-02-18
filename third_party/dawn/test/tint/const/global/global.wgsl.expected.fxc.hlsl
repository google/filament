struct tint_symbol {
  float4 value : SV_Target0;
};

float4 main_inner() {
  int v1 = 1;
  uint v2 = 1u;
  float v3 = 1.0f;
  int3 v4 = (1).xxx;
  uint3 v5 = (1u).xxx;
  float3 v6 = (1.0f).xxx;
  float3x3 v7 = float3x3((1.0f).xxx, (1.0f).xxx, (1.0f).xxx);
  float v9[10] = (float[10])0;
  return (0.0f).xxxx;
}

tint_symbol main() {
  float4 inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
