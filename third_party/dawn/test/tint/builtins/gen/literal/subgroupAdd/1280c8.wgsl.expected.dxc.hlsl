//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupAdd_1280c8() {
  uint2 res = WaveActiveSum((1u).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupAdd_1280c8()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupAdd_1280c8() {
  uint2 res = WaveActiveSum((1u).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupAdd_1280c8()));
  return;
}
