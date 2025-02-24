//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupMul_5a8c86() {
  int3 res = WaveActiveProduct((int(1)).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupMul_5a8c86()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupMul_5a8c86() {
  int3 res = WaveActiveProduct((int(1)).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupMul_5a8c86()));
}

