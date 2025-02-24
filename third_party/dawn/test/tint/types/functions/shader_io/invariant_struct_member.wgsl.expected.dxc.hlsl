struct Out {
  float4 pos;
};
struct tint_symbol {
  precise float4 pos : SV_Position;
};

Out main_inner() {
  Out tint_symbol_1 = (Out)0;
  return tint_symbol_1;
}

tint_symbol main() {
  Out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
