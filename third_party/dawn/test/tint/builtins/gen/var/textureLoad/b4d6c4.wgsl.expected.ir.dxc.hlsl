//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2D<float4> arg_0 : register(u0, space1);
float4 textureLoad_b4d6c4() {
  uint2 arg_1 = (1u).xx;
  float4 res = arg_0.Load(int3(int2(arg_1), int(0)));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_b4d6c4()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2D<float4> arg_0 : register(u0, space1);
float4 textureLoad_b4d6c4() {
  uint2 arg_1 = (1u).xx;
  float4 res = arg_0.Load(int3(int2(arg_1), int(0)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_b4d6c4()));
}

