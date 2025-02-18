//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupMin_2493ab() {
  uint res = WaveActiveMin(1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupMin_2493ab()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupMin_2493ab() {
  uint res = WaveActiveMin(1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupMin_2493ab()));
  return;
}
