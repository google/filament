//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2D<float4> arg_0 : register(u0, space1);

float4 textureLoad_39016c() {
  int2 arg_1 = (1).xx;
  float4 res = arg_0.Load(arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_39016c()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2D<float4> arg_0 : register(u0, space1);

float4 textureLoad_39016c() {
  int2 arg_1 = (1).xx;
  float4 res = arg_0.Load(arg_1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_39016c()));
  return;
}
