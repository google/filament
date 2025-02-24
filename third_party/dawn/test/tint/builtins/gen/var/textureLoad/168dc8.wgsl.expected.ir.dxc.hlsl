//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray<int4> arg_0 : register(t0, space1);
int4 textureLoad_168dc8() {
  uint2 arg_1 = (1u).xx;
  int arg_2 = int(1);
  int arg_3 = int(1);
  int v = arg_2;
  int v_1 = arg_3;
  int4 res = arg_0.Load(int4(int2(arg_1), v, v_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_168dc8()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray<int4> arg_0 : register(t0, space1);
int4 textureLoad_168dc8() {
  uint2 arg_1 = (1u).xx;
  int arg_2 = int(1);
  int arg_3 = int(1);
  int v = arg_2;
  int v_1 = arg_3;
  int4 res = arg_0.Load(int4(int2(arg_1), v, v_1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_168dc8()));
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
int4 textureLoad_168dc8() {
  uint2 arg_1 = (1u).xx;
  int arg_2 = int(1);
  int arg_3 = int(1);
  int v = arg_2;
  int v_1 = arg_3;
  int4 res = arg_0.Load(int4(int2(arg_1), v, v_1));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_2 = (VertexOutput)0;
  v_2.pos = (0.0f).xxxx;
  v_2.prevent_dce = textureLoad_168dc8();
  VertexOutput v_3 = v_2;
  return v_3;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_4 = vertex_main_inner();
  vertex_main_outputs v_5 = {v_4.prevent_dce, v_4.pos};
  return v_5;
}

