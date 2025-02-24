struct FragmentOutputs {
  float frag_depth;
  uint sample_mask;
};
struct tint_symbol {
  float frag_depth : SV_Depth;
  uint sample_mask : SV_Coverage;
};

FragmentOutputs main_inner() {
  FragmentOutputs tint_symbol_1 = {1.0f, 1u};
  return tint_symbol_1;
}

tint_symbol main() {
  FragmentOutputs inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.frag_depth = inner_result.frag_depth;
  wrapper_result.sample_mask = inner_result.sample_mask;
  return wrapper_result;
}
