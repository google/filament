struct FragInput {
  float4 a;
  float4 b;
};
struct FragOutput {
  float4 color;
  float4 blend;
};
struct tint_symbol_2 {
  float4 a : TEXCOORD0;
  float4 b : TEXCOORD1;
};
struct tint_symbol_3 {
  float4 color : SV_Target0;
  float4 blend : SV_Target1;
};

FragOutput frag_main_inner(FragInput tint_symbol) {
  FragOutput output = (FragOutput)0;
  output.color = tint_symbol.a;
  output.blend = tint_symbol.b;
  return output;
}

tint_symbol_3 frag_main(tint_symbol_2 tint_symbol_1) {
  FragInput tint_symbol_4 = {tint_symbol_1.a, tint_symbol_1.b};
  FragOutput inner_result = frag_main_inner(tint_symbol_4);
  tint_symbol_3 wrapper_result = (tint_symbol_3)0;
  wrapper_result.color = inner_result.color;
  wrapper_result.blend = inner_result.blend;
  return wrapper_result;
}
