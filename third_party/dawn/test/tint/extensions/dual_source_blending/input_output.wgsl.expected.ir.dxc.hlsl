struct FragOutput {
  float4 color;
  float4 blend;
};

struct FragInput {
  float4 a;
  float4 b;
};

struct frag_main_outputs {
  float4 FragOutput_color : SV_Target0;
  float4 FragOutput_blend : SV_Target1;
};

struct frag_main_inputs {
  float4 FragInput_a : TEXCOORD0;
  float4 FragInput_b : TEXCOORD1;
};


FragOutput frag_main_inner(FragInput v) {
  FragOutput output = (FragOutput)0;
  output.color = v.a;
  output.blend = v.b;
  FragOutput v_1 = output;
  return v_1;
}

frag_main_outputs frag_main(frag_main_inputs inputs) {
  FragInput v_2 = {inputs.FragInput_a, inputs.FragInput_b};
  FragOutput v_3 = frag_main_inner(v_2);
  frag_main_outputs v_4 = {v_3.color, v_3.blend};
  return v_4;
}

