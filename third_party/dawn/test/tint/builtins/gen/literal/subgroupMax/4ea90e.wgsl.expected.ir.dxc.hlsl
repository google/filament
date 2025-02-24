//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupMax_4ea90e() {
  int3 res = WaveActiveMax((int(1)).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupMax_4ea90e()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupMax_4ea90e() {
  int3 res = WaveActiveMax((int(1)).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupMax_4ea90e()));
}

