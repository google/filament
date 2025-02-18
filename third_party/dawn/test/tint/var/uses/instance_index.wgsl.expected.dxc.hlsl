struct tint_symbol_1 {
  uint b : SV_InstanceID;
};
struct tint_symbol_2 {
  float4 value : SV_Position;
};

float4 main_inner(uint b) {
  return float4((float(b)).xxxx);
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  float4 inner_result = main_inner(tint_symbol.b);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
