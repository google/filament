//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint3 subgroupAdd_6587ff() {
  uint3 res = WaveActiveSum((1u).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupAdd_6587ff()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint3 subgroupAdd_6587ff() {
  uint3 res = WaveActiveSum((1u).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupAdd_6587ff()));
  return;
}
