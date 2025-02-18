//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture1D<uint4> arg_0 : register(u0, space1);
uint4 textureLoad_17095b() {
  uint4 res = arg_0.Load(int2(int(1u), int(0)));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, textureLoad_17095b());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture1D<uint4> arg_0 : register(u0, space1);
uint4 textureLoad_17095b() {
  uint4 res = arg_0.Load(int2(int(1u), int(0)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, textureLoad_17095b());
}

