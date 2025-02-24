SKIP: INVALID

struct VertexInputs0 {
  uint vertex_index;
  int loc0;
};

struct VertexInputs1 {
  float loc2;
  float4 loc3;
  vector<float16_t, 3> loc5;
};

struct main_outputs {
  float4 tint_symbol : SV_Position;
};

struct main_inputs {
  int VertexInputs0_loc0 : TEXCOORD0;
  uint loc1 : TEXCOORD1;
  float VertexInputs1_loc2 : TEXCOORD2;
  float4 VertexInputs1_loc3 : TEXCOORD3;
  float16_t loc4 : TEXCOORD4;
  vector<float16_t, 3> VertexInputs1_loc5 : TEXCOORD5;
  uint VertexInputs0_vertex_index : SV_VertexID;
  uint instance_index : SV_InstanceID;
};


float4 main_inner(VertexInputs0 inputs0, uint loc1, uint instance_index, VertexInputs1 inputs1, float16_t loc4) {
  uint foo = (inputs0.vertex_index + instance_index);
  int i = inputs0.loc0;
  uint u = loc1;
  float f = inputs1.loc2;
  float4 v = inputs1.loc3;
  float16_t x = loc4;
  vector<float16_t, 3> y = inputs1.loc5;
  return (0.0f).xxxx;
}

main_outputs main(main_inputs inputs) {
  VertexInputs0 v_1 = {inputs.VertexInputs0_vertex_index, inputs.VertexInputs0_loc0};
  VertexInputs0 v_2 = v_1;
  VertexInputs1 v_3 = {inputs.VertexInputs1_loc2, inputs.VertexInputs1_loc3, inputs.VertexInputs1_loc5};
  main_outputs v_4 = {main_inner(v_2, inputs.loc1, inputs.instance_index, v_3, inputs.loc4)};
  return v_4;
}

FXC validation failure:
<scrubbed_path>(9,10-18): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
