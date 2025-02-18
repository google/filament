//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture3D<float4> arg_0 : register(u0, space1);
uint3 textureDimensions_cc947b() {
  uint3 v = (0u).xxx;
  arg_0.GetDimensions(v.x, v.y, v.z);
  uint3 res = v;
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, textureDimensions_cc947b());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture3D<float4> arg_0 : register(u0, space1);
uint3 textureDimensions_cc947b() {
  uint3 v = (0u).xxx;
  arg_0.GetDimensions(v.x, v.y, v.z);
  uint3 res = v;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, textureDimensions_cc947b());
}

