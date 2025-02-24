struct FragOutput {
  float4 color;
  float4 blend;
};
struct tint_symbol {
  float4 color : SV_Target0;
  float4 blend : SV_Target1;
};

FragOutput frag_main_inner() {
  FragOutput output = (FragOutput)0;
  output.color = float4(0.5f, 0.5f, 0.5f, 1.0f);
  output.blend = float4(0.5f, 0.5f, 0.5f, 1.0f);
  return output;
}

tint_symbol frag_main() {
  FragOutput inner_result = frag_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.color = inner_result.color;
  wrapper_result.blend = inner_result.blend;
  return wrapper_result;
}
