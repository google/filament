struct vs_main_outputs {
  float4 tint_symbol : SV_Position;
};

struct vs_main_inputs {
  uint in_vertex_index : SV_VertexID;
};


float4 vs_main_inner(uint in_vertex_index) {
  float4 v[3] = {float4(0.0f, 0.0f, 0.0f, 1.0f), float4(0.0f, 1.0f, 0.0f, 1.0f), float4(1.0f, 1.0f, 0.0f, 1.0f)};
  return v[min(in_vertex_index, 2u)];
}

vs_main_outputs vs_main(vs_main_inputs inputs) {
  vs_main_outputs v_1 = {vs_main_inner(inputs.in_vertex_index)};
  return v_1;
}

