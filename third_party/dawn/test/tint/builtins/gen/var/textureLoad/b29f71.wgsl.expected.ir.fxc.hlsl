//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray<int4> arg_0 : register(t0, space1);
int4 textureLoad_b29f71() {
  int2 arg_1 = (int(1)).xx;
  uint arg_2 = 1u;
  int arg_3 = int(1);
  int2 v = arg_1;
  int v_1 = arg_3;
  int4 res = arg_0.Load(int4(v, int(arg_2), v_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_b29f71()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray<int4> arg_0 : register(t0, space1);
int4 textureLoad_b29f71() {
  int2 arg_1 = (int(1)).xx;
  uint arg_2 = 1u;
  int arg_3 = int(1);
  int2 v = arg_1;
  int v_1 = arg_3;
  int4 res = arg_0.Load(int4(v, int(arg_2), v_1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_b29f71()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  int4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation int4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


Texture2DArray<int4> arg_0 : register(t0, space1);
int4 textureLoad_b29f71() {
  int2 arg_1 = (int(1)).xx;
  uint arg_2 = 1u;
  int arg_3 = int(1);
  int2 v = arg_1;
  int v_1 = arg_3;
  int4 res = arg_0.Load(int4(v, int(arg_2), v_1));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_2 = (VertexOutput)0;
  v_2.pos = (0.0f).xxxx;
  v_2.prevent_dce = textureLoad_b29f71();
  VertexOutput v_3 = v_2;
  return v_3;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_4 = vertex_main_inner();
  vertex_main_outputs v_5 = {v_4.prevent_dce, v_4.pos};
  return v_5;
}

