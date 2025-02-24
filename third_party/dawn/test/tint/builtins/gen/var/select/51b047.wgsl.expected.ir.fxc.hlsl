//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint2 select_51b047() {
  uint2 arg_0 = (1u).xx;
  uint2 arg_1 = (1u).xx;
  bool arg_2 = true;
  uint2 res = ((arg_2) ? (arg_1) : (arg_0));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, select_51b047());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint2 select_51b047() {
  uint2 arg_0 = (1u).xx;
  uint2 arg_1 = (1u).xx;
  bool arg_2 = true;
  uint2 res = ((arg_2) ? (arg_1) : (arg_0));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, select_51b047());
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  uint2 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint2 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


uint2 select_51b047() {
  uint2 arg_0 = (1u).xx;
  uint2 arg_1 = (1u).xx;
  bool arg_2 = true;
  uint2 res = ((arg_2) ? (arg_1) : (arg_0));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  v.prevent_dce = select_51b047();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.prevent_dce, v_2.pos};
  return v_3;
}

