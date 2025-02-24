struct VertexOutputs {
  float4 position;
};

struct main_outputs {
  float4 VertexOutputs_position : SV_Position;
};


VertexOutputs main_inner() {
  VertexOutputs v = {float4(1.0f, 2.0f, 3.0f, 4.0f)};
  return v;
}

main_outputs main() {
  VertexOutputs v_1 = main_inner();
  main_outputs v_2 = {v_1.position};
  return v_2;
}

