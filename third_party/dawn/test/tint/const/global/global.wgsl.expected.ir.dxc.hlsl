struct main_outputs {
  float4 tint_symbol : SV_Target0;
};


float4 main_inner() {
  int v1 = int(1);
  uint v2 = 1u;
  float v3 = 1.0f;
  int3 v4 = (int(1)).xxx;
  uint3 v5 = (1u).xxx;
  float3 v6 = (1.0f).xxx;
  float3x3 v7 = float3x3((1.0f).xxx, (1.0f).xxx, (1.0f).xxx);
  float v9[10] = (float[10])0;
  return (0.0f).xxxx;
}

main_outputs main() {
  main_outputs v = {main_inner()};
  return v;
}

