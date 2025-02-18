struct VertexOutputs {
  float clipDistance[6];
  float4 position;
};

struct main_outputs {
  float4 VertexOutputs_position : SV_Position;
  float4 VertexOutputs_clipDistance0 : SV_ClipDistance0;
  float2 VertexOutputs_clipDistance1 : SV_ClipDistance1;
};


VertexOutputs main_inner() {
  VertexOutputs v = {(float[6])0, float4(1.0f, 2.0f, 3.0f, 4.0f)};
  return v;
}

main_outputs main() {
  VertexOutputs v_1 = main_inner();
  float v_2[6] = v_1.clipDistance;
  float4 v_3 = float4(v_2[0u], v_2[1u], v_2[2u], v_2[3u]);
  main_outputs v_4 = {v_1.position, v_3, float2(v_2[4u], v_2[5u])};
  return v_4;
}

