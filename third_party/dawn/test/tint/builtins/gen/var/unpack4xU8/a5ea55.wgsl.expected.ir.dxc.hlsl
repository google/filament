//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 unpack4xU8_a5ea55() {
  uint arg_0 = 1u;
  uint4 res = unpack_u8u32(uint8_t4_packed(arg_0));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, unpack4xU8_a5ea55());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 unpack4xU8_a5ea55() {
  uint arg_0 = 1u;
  uint4 res = unpack_u8u32(uint8_t4_packed(arg_0));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, unpack4xU8_a5ea55());
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  uint4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


uint4 unpack4xU8_a5ea55() {
  uint arg_0 = 1u;
  uint4 res = unpack_u8u32(uint8_t4_packed(arg_0));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  v.prevent_dce = unpack4xU8_a5ea55();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.prevent_dce, v_2.pos};
  return v_3;
}

