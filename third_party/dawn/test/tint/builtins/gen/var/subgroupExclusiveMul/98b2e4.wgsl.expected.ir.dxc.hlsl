//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float subgroupExclusiveMul_98b2e4() {
  float arg_0 = 1.0f;
  float res = WavePrefixProduct(arg_0);
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
  float arg_0 = 1.0f;
  float res = WavePrefixProduct(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveMul_98b2e4()));
}

