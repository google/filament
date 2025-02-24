//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float subgroupExclusiveMul_98b2e4() {
  float res = WavePrefixProduct(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveMul_98b2e4()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float subgroupExclusiveMul_98b2e4() {
  float res = WavePrefixProduct(1.0f);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveMul_98b2e4()));
}

