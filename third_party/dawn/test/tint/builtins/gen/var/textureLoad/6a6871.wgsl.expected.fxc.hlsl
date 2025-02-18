//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);
RWTexture3D<float4> arg_0 : register(u0, space1);

float4 textureLoad_6a6871() {
  int3 arg_1 = (1).xxx;
  float4 res = arg_0.Load(arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_6a6871()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);
RWTexture3D<float4> arg_0 : register(u0, space1);

float4 textureLoad_6a6871() {
  int3 arg_1 = (1).xxx;
  float4 res = arg_0.Load(arg_1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_6a6871()));
  return;
}
