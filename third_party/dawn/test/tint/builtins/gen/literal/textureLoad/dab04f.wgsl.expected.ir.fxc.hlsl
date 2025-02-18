//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2DArray<float4> arg_0 : register(u0, space1);
float4 textureLoad_dab04f() {
  float4 res = arg_0.Load(int4((int(1)).xx, int(1), int(0)));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_dab04f()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2DArray<float4> arg_0 : register(u0, space1);
float4 textureLoad_dab04f() {
  float4 res = arg_0.Load(int4((int(1)).xx, int(1), int(0)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_dab04f()));
}

