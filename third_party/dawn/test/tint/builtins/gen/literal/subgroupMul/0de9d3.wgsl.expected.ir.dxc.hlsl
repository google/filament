//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float subgroupMul_0de9d3() {
  float res = WaveActiveProduct(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupMul_0de9d3()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float subgroupMul_0de9d3() {
  float res = WaveActiveProduct(1.0f);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupMul_0de9d3()));
}

