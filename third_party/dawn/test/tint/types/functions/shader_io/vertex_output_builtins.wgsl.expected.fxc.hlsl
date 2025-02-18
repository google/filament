struct tint_symbol {
  float4 value : SV_Position;
};

float4 main_inner() {
  return float4(1.0f, 2.0f, 3.0f, 4.0f);
}

tint_symbol main() {
  float4 inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
