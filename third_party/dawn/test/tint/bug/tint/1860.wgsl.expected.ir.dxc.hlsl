struct main_outputs {
  float4 tint_symbol : SV_Position;
};


cbuffer cbuffer_declared_after_usage : register(b0) {
  uint4 declared_after_usage[1];
};
float4 main_inner() {
  return float4((asfloat(declared_after_usage[0u].x)).xxxx);
}

main_outputs main() {
  main_outputs v = {main_inner()};
  return v;
}

