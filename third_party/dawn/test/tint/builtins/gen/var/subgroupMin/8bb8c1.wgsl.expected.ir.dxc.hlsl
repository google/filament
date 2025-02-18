//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint2 subgroupMin_8bb8c1() {
  uint2 arg_0 = (1u).xx;
  uint2 res = WaveActiveMin(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, subgroupMin_8bb8c1());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint2 subgroupMin_8bb8c1() {
  uint2 arg_0 = (1u).xx;
  uint2 res = WaveActiveMin(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, subgroupMin_8bb8c1());
}

