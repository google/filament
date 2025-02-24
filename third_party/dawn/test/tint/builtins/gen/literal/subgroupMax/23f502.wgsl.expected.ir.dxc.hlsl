//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupMax_23f502() {
  uint3 res = WaveActiveMax((1u).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, subgroupMax_23f502());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupMax_23f502() {
  uint3 res = WaveActiveMax((1u).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, subgroupMax_23f502());
}

