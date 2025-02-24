struct FragOutput {
  float4 color;
  float4 blend;
};

struct frag_main_outputs {
  float4 FragOutput_color : SV_Target0;
  float4 FragOutput_blend : SV_Target1;
};


FragOutput frag_main_inner() {
  FragOutput output = (FragOutput)0;
  output.color = float4(0.5f, 0.5f, 0.5f, 1.0f);
  output.blend = float4(0.5f, 0.5f, 0.5f, 1.0f);
  FragOutput v = output;
  return v;
}

frag_main_outputs frag_main() {
  FragOutput v_1 = frag_main_inner();
  frag_main_outputs v_2 = {v_1.color, v_1.blend};
  return v_2;
}

