struct VertexOutputs {
  int loc0;
  uint loc1;
  float loc2;
  float4 loc3;
  float4 position;
};

struct main_outputs {
  nointerpolation int VertexOutputs_loc0 : TEXCOORD0;
  nointerpolation uint VertexOutputs_loc1 : TEXCOORD1;
  float VertexOutputs_loc2 : TEXCOORD2;
  float4 VertexOutputs_loc3 : TEXCOORD3;
  float4 VertexOutputs_position : SV_Position;
};


VertexOutputs main_inner() {
  VertexOutputs v = {int(1), 1u, 1.0f, float4(1.0f, 2.0f, 3.0f, 4.0f), (0.0f).xxxx};
  return v;
}

main_outputs main() {
  VertexOutputs v_1 = main_inner();
  main_outputs v_2 = {v_1.loc0, v_1.loc1, v_1.loc2, v_1.loc3, v_1.position};
  return v_2;
}

