//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2DArray<int4> arg_0 : register(u0, space1);

int4 textureLoad_c9b083() {
  int2 arg_1 = (1).xx;
  uint arg_2 = 1u;
  int4 res = arg_0.Load(int3(arg_1, int(arg_2)));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_c9b083()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2DArray<int4> arg_0 : register(u0, space1);

int4 textureLoad_c9b083() {
  int2 arg_1 = (1).xx;
  uint arg_2 = 1u;
  int4 res = arg_0.Load(int3(arg_1, int(arg_2)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_c9b083()));
  return;
}
