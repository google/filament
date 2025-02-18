//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture1D<float4> arg_0 : register(u0, space1);
float4 textureLoad_92dd61() {
  uint arg_1 = 1u;
  float4 res = arg_0.Load(int2(int(arg_1), int(0)));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_92dd61()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture1D<float4> arg_0 : register(u0, space1);
float4 textureLoad_92dd61() {
  uint arg_1 = 1u;
  float4 res = arg_0.Load(int2(int(arg_1), int(0)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_92dd61()));
}

