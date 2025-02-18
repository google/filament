//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2DArray<float4> arg_0 : register(u0, space1);
float4 textureLoad_bc882d() {
  uint2 arg_1 = (1u).xx;
  int arg_2 = int(1);
  int v = arg_2;
  float4 res = arg_0.Load(int4(int2(arg_1), v, int(0)));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_bc882d()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2DArray<float4> arg_0 : register(u0, space1);
float4 textureLoad_bc882d() {
  uint2 arg_1 = (1u).xx;
  int arg_2 = int(1);
  int v = arg_2;
  float4 res = arg_0.Load(int4(int2(arg_1), v, int(0)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_bc882d()));
}

