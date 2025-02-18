SKIP: INVALID

struct VertexInputs {
  int loc0;
  uint loc1;
  float loc2;
  float4 loc3;
  float16_t loc4;
  vector<float16_t, 3> loc5;
};

struct main_outputs {
  float4 tint_symbol : SV_Position;
};

struct main_inputs {
  int VertexInputs_loc0 : TEXCOORD0;
  uint VertexInputs_loc1 : TEXCOORD1;
  float VertexInputs_loc2 : TEXCOORD2;
  float4 VertexInputs_loc3 : TEXCOORD3;
  float16_t VertexInputs_loc4 : TEXCOORD4;
  vector<float16_t, 3> VertexInputs_loc5 : TEXCOORD5;
};


float4 main_inner(VertexInputs inputs) {
  int i = inputs.loc0;
  uint u = inputs.loc1;
  float f = inputs.loc2;
  float4 v = inputs.loc3;
  float16_t x = inputs.loc4;
  vector<float16_t, 3> y = inputs.loc5;
  return (0.0f).xxxx;
}

main_outputs main(main_inputs inputs) {
  VertexInputs v_1 = {inputs.VertexInputs_loc0, inputs.VertexInputs_loc1, inputs.VertexInputs_loc2, inputs.VertexInputs_loc3, inputs.VertexInputs_loc4, inputs.VertexInputs_loc5};
  main_outputs v_2 = {main_inner(v_1)};
  return v_2;
}

FXC validation failure:
<scrubbed_path>(6,3-11): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
