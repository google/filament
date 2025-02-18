struct tint_symbol {
  float4 value : SV_Target0;
};

float4 main_inner() {
  return (0.0f).xxxx;
}

tint_symbol main() {
  float4 inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
