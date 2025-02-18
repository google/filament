//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float2 subgroupMax_1fc846() {
  float2 arg_0 = (1.0f).xx;
  float2 res = WaveActiveMax(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupMax_1fc846()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float2 subgroupMax_1fc846() {
  float2 arg_0 = (1.0f).xx;
  float2 res = WaveActiveMax(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupMax_1fc846()));
}

