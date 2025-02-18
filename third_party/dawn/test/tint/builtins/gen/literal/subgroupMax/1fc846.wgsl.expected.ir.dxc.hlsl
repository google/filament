//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float2 subgroupMax_1fc846() {
  float2 res = WaveActiveMax((1.0f).xx);
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
  float2 res = WaveActiveMax((1.0f).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupMax_1fc846()));
}

