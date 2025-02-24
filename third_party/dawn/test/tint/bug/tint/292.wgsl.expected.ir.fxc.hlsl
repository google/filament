struct main_outputs {
  float4 tint_symbol : SV_Position;
};


float4 main_inner() {
  float3 light = float3(1.20000004768371582031f, 1.0f, 2.0f);
  float3 negative_light = -(light);
  return (0.0f).xxxx;
}

main_outputs main() {
  main_outputs v = {main_inner()};
  return v;
}

