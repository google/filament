//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int subgroupMul_3fe886() {
  int res = WaveActiveProduct(1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupMul_3fe886()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int subgroupMul_3fe886() {
  int res = WaveActiveProduct(1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupMul_3fe886()));
  return;
}
