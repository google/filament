SKIP: INVALID

struct VertexOutputs {
  int loc0;
  uint loc1;
  float loc2;
  float4 loc3;
  float4 position;
  float16_t loc4;
  vector<float16_t, 3> loc5;
};

struct main_outputs {
  nointerpolation int VertexOutputs_loc0 : TEXCOORD0;
  nointerpolation uint VertexOutputs_loc1 : TEXCOORD1;
  float VertexOutputs_loc2 : TEXCOORD2;
  float4 VertexOutputs_loc3 : TEXCOORD3;
  float16_t VertexOutputs_loc4 : TEXCOORD4;
  vector<float16_t, 3> VertexOutputs_loc5 : TEXCOORD5;
  float4 VertexOutputs_position : SV_Position;
};


VertexOutputs main_inner() {
  VertexOutputs v = {int(1), 1u, 1.0f, float4(1.0f, 2.0f, 3.0f, 4.0f), (0.0f).xxxx, float16_t(2.25h), vector<float16_t, 3>(float16_t(3.0h), float16_t(5.0h), float16_t(8.0h))};
  return v;
}

main_outputs main() {
  VertexOutputs v_1 = main_inner();
  VertexOutputs v_2 = v_1;
  VertexOutputs v_3 = v_1;
  VertexOutputs v_4 = v_1;
  VertexOutputs v_5 = v_1;
  VertexOutputs v_6 = v_1;
  VertexOutputs v_7 = v_1;
  VertexOutputs v_8 = v_1;
  main_outputs v_9 = {v_2.loc0, v_3.loc1, v_4.loc2, v_5.loc3, v_7.loc4, v_8.loc5, v_6.position};
  return v_9;
}

FXC validation failure:
<scrubbed_path>(7,3-11): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
