struct main_outputs {
  float4 tint_symbol : SV_Position;
};

struct main_inputs {
  uint vertex_index : SV_VertexID;
  uint instance_index : SV_InstanceID;
};


float4 main_inner(uint vertex_index, uint instance_index) {
  uint foo = (vertex_index + instance_index);
  return (0.0f).xxxx;
}

main_outputs main(main_inputs inputs) {
  main_outputs v = {main_inner(inputs.vertex_index, inputs.instance_index)};
  return v;
}

