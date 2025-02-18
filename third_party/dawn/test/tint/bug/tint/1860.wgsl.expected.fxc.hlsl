cbuffer cbuffer_declared_after_usage : register(b0) {
  uint4 declared_after_usage[1];
};

struct tint_symbol {
  float4 value : SV_Position;
};

float4 main_inner() {
  return float4((asfloat(declared_after_usage[0].x)).xxxx);
}

tint_symbol main() {
  float4 inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
