//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int4 subgroupMax_a3d5f7() {
  int4 res = WaveActiveMax((1).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupMax_a3d5f7()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int4 subgroupMax_a3d5f7() {
  int4 res = WaveActiveMax((1).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupMax_a3d5f7()));
  return;
}
