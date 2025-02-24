//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 unpack4x8snorm_523fb3() {
  uint arg_0 = 1u;
  int v = int(arg_0);
  float4 res = clamp((float4((int4((v << 24u), (v << 16u), (v << 8u), v) >> (24u).xxxx)) / 127.0f), (-1.0f).xxxx, (1.0f).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(unpack4x8snorm_523fb3()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 unpack4x8snorm_523fb3() {
  uint arg_0 = 1u;
  int v = int(arg_0);
  float4 res = clamp((float4((int4((v << 24u), (v << 16u), (v << 8u), v) >> (24u).xxxx)) / 127.0f), (-1.0f).xxxx, (1.0f).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(unpack4x8snorm_523fb3()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  float4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation float4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


float4 unpack4x8snorm_523fb3() {
  uint arg_0 = 1u;
  int v = int(arg_0);
  float4 res = clamp((float4((int4((v << 24u), (v << 16u), (v << 8u), v) >> (24u).xxxx)) / 127.0f), (-1.0f).xxxx, (1.0f).xxxx);
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_1 = (VertexOutput)0;
  v_1.pos = (0.0f).xxxx;
  v_1.prevent_dce = unpack4x8snorm_523fb3();
  VertexOutput v_2 = v_1;
  return v_2;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_3 = vertex_main_inner();
  vertex_main_outputs v_4 = {v_3.prevent_dce, v_3.pos};
  return v_4;
}

