struct VertexOutputs {
  float clipDistance[1];
  float4 position;
};

struct main_outputs {
  float4 VertexOutputs_position : SV_Position;
  float VertexOutputs_clipDistance0 : SV_ClipDistance0;
};


VertexOutputs main_inner() {
  VertexOutputs v = {(float[1])0, float4(1.0f, 2.0f, 3.0f, 4.0f)};
  return v;
}

main_outputs main() {
  VertexOutputs v_1 = main_inner();
  float v_2[1] = v_1.clipDistance;
  main_outputs v_3 = {v_1.position, v_2[0u]};
  return v_3;
}

