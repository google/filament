struct VertexInputs {
  int loc0;
  uint loc1;
  float loc2;
  float4 loc3;
};

struct main_outputs {
  float4 tint_symbol : SV_Position;
};

struct main_inputs {
  int VertexInputs_loc0 : TEXCOORD0;
  uint VertexInputs_loc1 : TEXCOORD1;
  float VertexInputs_loc2 : TEXCOORD2;
  float4 VertexInputs_loc3 : TEXCOORD3;
};


float4 main_inner(VertexInputs inputs) {
  int i = inputs.loc0;
  uint u = inputs.loc1;
  float f = inputs.loc2;
  float4 v = inputs.loc3;
  return (0.0f).xxxx;
}

main_outputs main(main_inputs inputs) {
  VertexInputs v_1 = {inputs.VertexInputs_loc0, inputs.VertexInputs_loc1, inputs.VertexInputs_loc2, inputs.VertexInputs_loc3};
  main_outputs v_2 = {main_inner(v_1)};
  return v_2;
}

