struct main_outputs {
  float4 tint_symbol : SV_Position;
};

struct main_inputs {
  uint VertexIndex : SV_VertexID;
};


float4 main_inner(uint VertexIndex) {
  return float4(0.0f, 0.0f, 0.0f, 1.0f);
}

main_outputs main(main_inputs inputs) {
  main_outputs v = {main_inner(inputs.VertexIndex)};
  return v;
}

