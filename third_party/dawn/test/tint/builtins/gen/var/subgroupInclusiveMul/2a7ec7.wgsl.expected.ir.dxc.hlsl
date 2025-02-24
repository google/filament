//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float subgroupInclusiveMul_2a7ec7() {
  float arg_0 = 1.0f;
  float v = arg_0;
  float res = (WavePrefixProduct(v) * v);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupInclusiveMul_2a7ec7()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float subgroupInclusiveMul_2a7ec7() {
  float arg_0 = 1.0f;
  float v = arg_0;
  float res = (WavePrefixProduct(v) * v);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupInclusiveMul_2a7ec7()));
}

