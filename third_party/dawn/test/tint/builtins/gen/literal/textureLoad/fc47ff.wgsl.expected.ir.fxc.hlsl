//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2D<int4> arg_0 : register(u0, space1);
int4 textureLoad_fc47ff() {
  int4 res = arg_0.Load(int3(int2((1u).xx), int(0)));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_fc47ff()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2D<int4> arg_0 : register(u0, space1);
int4 textureLoad_fc47ff() {
  int4 res = arg_0.Load(int3(int2((1u).xx), int(0)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_fc47ff()));
}

