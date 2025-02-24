//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture1D<uint4> arg_0 : register(u0, space1);
uint4 textureLoad_b60a86() {
  int arg_1 = int(1);
  uint4 res = arg_0.Load(int2(arg_1, int(0)));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, textureLoad_b60a86());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture1D<uint4> arg_0 : register(u0, space1);
uint4 textureLoad_b60a86() {
  int arg_1 = int(1);
  uint4 res = arg_0.Load(int2(arg_1, int(0)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, textureLoad_b60a86());
}

