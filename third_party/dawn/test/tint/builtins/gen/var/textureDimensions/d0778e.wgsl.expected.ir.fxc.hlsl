//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture3D<uint4> arg_0 : register(t0, space1);
uint3 textureDimensions_d0778e() {
  uint3 v = (0u).xxx;
  arg_0.GetDimensions(v.x, v.y, v.z);
  uint3 res = v;
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, textureDimensions_d0778e());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture3D<uint4> arg_0 : register(t0, space1);
uint3 textureDimensions_d0778e() {
  uint3 v = (0u).xxx;
  arg_0.GetDimensions(v.x, v.y, v.z);
  uint3 res = v;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, textureDimensions_d0778e());
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  uint3 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint3 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


Texture3D<uint4> arg_0 : register(t0, space1);
uint3 textureDimensions_d0778e() {
  uint3 v = (0u).xxx;
  arg_0.GetDimensions(v.x, v.y, v.z);
  uint3 res = v;
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_1 = (VertexOutput)0;
  v_1.pos = (0.0f).xxxx;
  v_1.prevent_dce = textureDimensions_d0778e();
  VertexOutput v_2 = v_1;
  return v_2;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_3 = vertex_main_inner();
  vertex_main_outputs v_4 = {v_3.prevent_dce, v_3.pos};
  return v_4;
}

