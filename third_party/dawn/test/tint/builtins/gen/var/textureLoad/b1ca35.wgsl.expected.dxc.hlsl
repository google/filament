//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2DArray<int4> arg_0 : register(u0, space1);

int4 textureLoad_b1ca35() {
  uint2 arg_1 = (1u).xx;
  uint arg_2 = 1u;
  int4 res = arg_0.Load(uint3(arg_1, arg_2));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_b1ca35()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2DArray<int4> arg_0 : register(u0, space1);

int4 textureLoad_b1ca35() {
  uint2 arg_1 = (1u).xx;
  uint arg_2 = 1u;
  int4 res = arg_0.Load(uint3(arg_1, arg_2));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_b1ca35()));
  return;
}
