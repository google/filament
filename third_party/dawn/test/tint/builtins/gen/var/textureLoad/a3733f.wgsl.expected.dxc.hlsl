//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2D<uint4> arg_0 : register(u0, space1);

uint4 textureLoad_a3733f() {
  int2 arg_1 = (1).xx;
  uint4 res = arg_0.Load(arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_a3733f()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2D<uint4> arg_0 : register(u0, space1);

uint4 textureLoad_a3733f() {
  int2 arg_1 = (1).xx;
  uint4 res = arg_0.Load(arg_1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_a3733f()));
  return;
}
