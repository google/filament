struct VertexInputs0 {
  uint vertex_index;
  int loc0;
};
struct VertexInputs1 {
  float loc2;
  float4 loc3;
};
struct tint_symbol_1 {
  int loc0 : TEXCOORD0;
  uint loc1 : TEXCOORD1;
  float loc2 : TEXCOORD2;
  float4 loc3 : TEXCOORD3;
  uint vertex_index : SV_VertexID;
  uint instance_index : SV_InstanceID;
};
struct tint_symbol_2 {
  float4 value : SV_Position;
};

float4 main_inner(VertexInputs0 inputs0, uint loc1, uint instance_index, VertexInputs1 inputs1) {
  uint foo = (inputs0.vertex_index + instance_index);
  int i = inputs0.loc0;
  uint u = loc1;
  float f = inputs1.loc2;
  float4 v = inputs1.loc3;
  return (0.0f).xxxx;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  VertexInputs0 tint_symbol_3 = {tint_symbol.vertex_index, tint_symbol.loc0};
  VertexInputs1 tint_symbol_4 = {tint_symbol.loc2, tint_symbol.loc3};
  float4 inner_result = main_inner(tint_symbol_3, tint_symbol.loc1, tint_symbol.instance_index, tint_symbol_4);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
