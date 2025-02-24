//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2D<uint4> arg_0 : register(u0, space1);
uint4 textureLoad_170593() {
  int2 arg_1 = (int(1)).xx;
  uint4 res = arg_0.Load(int3(arg_1, int(0)));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, textureLoad_170593());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2D<uint4> arg_0 : register(u0, space1);
uint4 textureLoad_170593() {
  int2 arg_1 = (int(1)).xx;
  uint4 res = arg_0.Load(int3(arg_1, int(0)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, textureLoad_170593());
}

