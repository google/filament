//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2DArray<float4> arg_0 : register(u0, space1);

float4 textureLoad_7dd3d5() {
  int2 arg_1 = (1).xx;
  int arg_2 = 1;
  float4 res = arg_0.Load(int3(arg_1, arg_2));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_7dd3d5()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2DArray<float4> arg_0 : register(u0, space1);

float4 textureLoad_7dd3d5() {
  int2 arg_1 = (1).xx;
  int arg_2 = 1;
  float4 res = arg_0.Load(int3(arg_1, arg_2));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_7dd3d5()));
  return;
}
