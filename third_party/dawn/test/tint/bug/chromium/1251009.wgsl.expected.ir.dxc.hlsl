struct VertexInputs0 {
  uint vertex_index;
  int loc0;
};

struct VertexInputs1 {
  uint loc1;
  float4 loc3;
};

struct main_outputs {
  float4 tint_symbol : SV_Position;
};

struct main_inputs {
  int VertexInputs0_loc0 : TEXCOORD0;
  uint loc1 : TEXCOORD1;
  uint VertexInputs1_loc1 : TEXCOORD2;
  float4 VertexInputs1_loc3 : TEXCOORD3;
  uint VertexInputs0_vertex_index : SV_VertexID;
  uint instance_index : SV_InstanceID;
};


float4 main_inner(VertexInputs0 inputs0, uint loc1, uint instance_index, VertexInputs1 inputs1) {
  uint foo = (inputs0.vertex_index + instance_index);
  return (0.0f).xxxx;
}

main_outputs main(main_inputs inputs) {
  VertexInputs0 v = {inputs.VertexInputs0_vertex_index, inputs.VertexInputs0_loc0};
  VertexInputs1 v_1 = {inputs.VertexInputs1_loc1, inputs.VertexInputs1_loc3};
  main_outputs v_2 = {main_inner(v, inputs.loc1, inputs.instance_index, v_1)};
  return v_2;
}

