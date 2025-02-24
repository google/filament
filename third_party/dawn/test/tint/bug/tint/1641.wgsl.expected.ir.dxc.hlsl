struct Normals {
  float3 f;
};

struct main_outputs {
  float4 tint_symbol : SV_Position;
};


float4 main_inner() {
  int zero = int(0);
  Normals v[1] = {{float3(0.0f, 0.0f, 1.0f)}};
  return float4(v[min(uint(zero), 0u)].f, 1.0f);
}

main_outputs main() {
  main_outputs v_1 = {main_inner()};
  return v_1;
}

