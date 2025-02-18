struct VertexInputs {
  uint vertex_index;
  uint instance_index;
};

struct main_outputs {
  float4 tint_symbol : SV_Position;
};

struct main_inputs {
  uint VertexInputs_vertex_index : SV_VertexID;
  uint VertexInputs_instance_index : SV_InstanceID;
};


float4 main_inner(VertexInputs inputs) {
  uint foo = (inputs.vertex_index + inputs.instance_index);
  return (0.0f).xxxx;
}

main_outputs main(main_inputs inputs) {
  VertexInputs v = {inputs.VertexInputs_vertex_index, inputs.VertexInputs_instance_index};
  main_outputs v_1 = {main_inner(v)};
  return v_1;
}

