//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture1D<uint4> arg_0 : register(t0, space1);
uint textureDimensions_7d8439() {
  uint v = 0u;
  arg_0.GetDimensions(v);
  uint res = v;
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, textureDimensions_7d8439());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture1D<uint4> arg_0 : register(t0, space1);
uint textureDimensions_7d8439() {
  uint v = 0u;
  arg_0.GetDimensions(v);
  uint res = v;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, textureDimensions_7d8439());
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


Texture1D<uint4> arg_0 : register(t0, space1);
uint textureDimensions_7d8439() {
  uint v = 0u;
  arg_0.GetDimensions(v);
  uint res = v;
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_1 = (VertexOutput)0;
  v_1.pos = (0.0f).xxxx;
  v_1.prevent_dce = textureDimensions_7d8439();
  VertexOutput v_2 = v_1;
  return v_2;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_3 = vertex_main_inner();
  vertex_main_outputs v_4 = {v_3.prevent_dce, v_3.pos};
  return v_4;
}

