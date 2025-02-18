//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);
RWTexture3D<float4> arg_0 : register(u0, space1);

float4 textureLoad_272e7a() {
  uint3 arg_1 = (1u).xxx;
  float4 res = arg_0.Load(arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_272e7a()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);
RWTexture3D<float4> arg_0 : register(u0, space1);

float4 textureLoad_272e7a() {
  uint3 arg_1 = (1u).xxx;
  float4 res = arg_0.Load(arg_1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_272e7a()));
  return;
}
