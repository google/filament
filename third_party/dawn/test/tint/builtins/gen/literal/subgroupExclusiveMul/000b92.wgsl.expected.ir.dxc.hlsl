//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupExclusiveMul_000b92() {
  uint4 res = WavePrefixProduct((1u).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, subgroupExclusiveMul_000b92());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupExclusiveMul_000b92() {
  uint4 res = WavePrefixProduct((1u).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, subgroupExclusiveMul_000b92());
}

