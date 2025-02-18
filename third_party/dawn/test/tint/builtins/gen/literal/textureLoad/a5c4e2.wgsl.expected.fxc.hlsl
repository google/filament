//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);
RWTexture1D<uint4> arg_0 : register(u0, space1);

uint4 textureLoad_a5c4e2() {
  uint4 res = arg_0.Load(1);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_a5c4e2()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);
RWTexture1D<uint4> arg_0 : register(u0, space1);

uint4 textureLoad_a5c4e2() {
  uint4 res = arg_0.Load(1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_a5c4e2()));
  return;
}
