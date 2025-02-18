struct Normals {
  float3 f;
};

struct tint_symbol {
  float4 value : SV_Position;
};

float4 main_inner() {
  int zero = 0;
  Normals tint_symbol_1[1] = {{float3(0.0f, 0.0f, 1.0f)}};
  return float4(tint_symbol_1[min(uint(zero), 0u)].f, 1.0f);
}

tint_symbol main() {
  float4 inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
