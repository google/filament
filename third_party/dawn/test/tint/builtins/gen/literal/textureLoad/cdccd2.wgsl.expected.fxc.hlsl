//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2DArray<uint4> arg_0 : register(u0, space1);

uint4 textureLoad_cdccd2() {
  uint4 res = arg_0.Load(int3((1).xx, int(1u)));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_cdccd2()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2DArray<uint4> arg_0 : register(u0, space1);

uint4 textureLoad_cdccd2() {
  uint4 res = arg_0.Load(int3((1).xx, int(1u)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_cdccd2()));
  return;
}
