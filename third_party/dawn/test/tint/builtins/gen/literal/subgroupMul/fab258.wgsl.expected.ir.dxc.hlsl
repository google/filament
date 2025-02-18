//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupMul_fab258() {
  int4 res = WaveActiveProduct((int(1)).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupMul_fab258()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupMul_fab258() {
  int4 res = WaveActiveProduct((int(1)).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupMul_fab258()));
}

