//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint pack4xI8Clamp_e42b2a() {
  int4 arg_0 = (int(1)).xxxx;
  int4 v = arg_0;
  uint4 v_1 = uint4(0u, 8u, 16u, 24u);
  int4 v_2 = int4((int(-128)).xxxx);
  uint4 v_3 = asuint(clamp(v, v_2, int4((int(127)).xxxx)));
  uint4 v_4 = ((v_3 & uint4((255u).xxxx)) << v_1);
  uint res = dot(v_4, uint4((1u).xxxx));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, pack4xI8Clamp_e42b2a());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint pack4xI8Clamp_e42b2a() {
  int4 arg_0 = (int(1)).xxxx;
  int4 v = arg_0;
  uint4 v_1 = uint4(0u, 8u, 16u, 24u);
  int4 v_2 = int4((int(-128)).xxxx);
  uint4 v_3 = asuint(clamp(v, v_2, int4((int(127)).xxxx)));
  uint4 v_4 = ((v_3 & uint4((255u).xxxx)) << v_1);
  uint res = dot(v_4, uint4((1u).xxxx));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, pack4xI8Clamp_e42b2a());
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  uint prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


uint pack4xI8Clamp_e42b2a() {
  int4 arg_0 = (int(1)).xxxx;
  int4 v = arg_0;
  uint4 v_1 = uint4(0u, 8u, 16u, 24u);
  int4 v_2 = int4((int(-128)).xxxx);
  uint4 v_3 = asuint(clamp(v, v_2, int4((int(127)).xxxx)));
  uint4 v_4 = ((v_3 & uint4((255u).xxxx)) << v_1);
  uint res = dot(v_4, uint4((1u).xxxx));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_5 = (VertexOutput)0;
  v_5.pos = (0.0f).xxxx;
  v_5.prevent_dce = pack4xI8Clamp_e42b2a();
  VertexOutput v_6 = v_5;
  return v_6;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_7 = vertex_main_inner();
  vertex_main_outputs v_8 = {v_7.prevent_dce, v_7.pos};
  return v_8;
}

