//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupMax_b58cbf() {
  uint arg_0 = 1u;
  uint res = WaveActiveMax(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupMax_b58cbf()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupMax_b58cbf() {
  uint arg_0 = 1u;
  uint res = WaveActiveMax(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupMax_b58cbf()));
  return;
}
