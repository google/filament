struct VertexInputs {
  uint vertex_index;
  uint instance_index;
};
struct tint_symbol_1 {
  uint vertex_index : SV_VertexID;
  uint instance_index : SV_InstanceID;
};
struct tint_symbol_2 {
  float4 value : SV_Position;
};

float4 main_inner(VertexInputs inputs) {
  uint foo = (inputs.vertex_index + inputs.instance_index);
  return (0.0f).xxxx;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  VertexInputs tint_symbol_3 = {tint_symbol.vertex_index, tint_symbol.instance_index};
  float4 inner_result = main_inner(tint_symbol_3);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
