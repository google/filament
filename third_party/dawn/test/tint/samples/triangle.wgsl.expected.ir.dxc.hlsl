//
// vtx_main
//
struct vtx_main_outputs {
  float4 tint_symbol : SV_Position;
};

struct vtx_main_inputs {
  uint VertexIndex : SV_VertexID;
};


float4 vtx_main_inner(uint VertexIndex) {
  float2 v[3] = {float2(0.0f, 0.5f), (-0.5f).xx, float2(0.5f, -0.5f)};
  return float4(v[min(VertexIndex, 2u)], 0.0f, 1.0f);
}

vtx_main_outputs vtx_main(vtx_main_inputs inputs) {
  vtx_main_outputs v_1 = {vtx_main_inner(inputs.VertexIndex)};
  return v_1;
}

//
// frag_main
//
struct frag_main_outputs {
  float4 tint_symbol : SV_Target0;
};


float4 frag_main_inner() {
  return float4(1.0f, 0.0f, 0.0f, 1.0f);
}

frag_main_outputs frag_main() {
  frag_main_outputs v = {frag_main_inner()};
  return v;
}

