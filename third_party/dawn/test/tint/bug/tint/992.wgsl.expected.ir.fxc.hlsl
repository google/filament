struct frag_main_outputs {
  float4 tint_symbol : SV_Target0;
};


float4 frag_main_inner() {
  float b = 0.0f;
  float3 v = float3((b).xxx);
  return float4(v, 1.0f);
}

frag_main_outputs frag_main() {
  frag_main_outputs v_1 = {frag_main_inner()};
  return v_1;
}

