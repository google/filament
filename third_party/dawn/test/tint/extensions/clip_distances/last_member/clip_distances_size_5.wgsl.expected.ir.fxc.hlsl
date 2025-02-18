struct VertexOutputs {
  float4 position;
  float clipDistance[5];
};

struct main_outputs {
  float4 VertexOutputs_position : SV_Position;
  float4 VertexOutputs_clipDistance0 : SV_ClipDistance0;
  float VertexOutputs_clipDistance1 : SV_ClipDistance1;
};


VertexOutputs main_inner() {
  VertexOutputs v = {float4(1.0f, 2.0f, 3.0f, 4.0f), (float[5])0};
  return v;
}

main_outputs main() {
  VertexOutputs v_1 = main_inner();
  float v_2[5] = v_1.clipDistance;
  main_outputs v_3 = {v_1.position, float4(v_2[0u], v_2[1u], v_2[2u], v_2[3u]), v_2[4u]};
  return v_3;
}

