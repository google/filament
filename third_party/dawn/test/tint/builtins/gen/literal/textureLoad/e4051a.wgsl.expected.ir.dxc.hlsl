//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2D<float4> arg_0 : register(u0, space1);
float4 textureLoad_e4051a() {
  float4 res = arg_0.Load(int3(int2((1u).xx), int(0)));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_e4051a()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2D<float4> arg_0 : register(u0, space1);
float4 textureLoad_e4051a() {
  float4 res = arg_0.Load(int3(int2((1u).xx), int(0)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_e4051a()));
}

